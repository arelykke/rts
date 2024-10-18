#include "philo.h"

void thinking(t_philo *philo, bool pre_simulation)
{
    long time_eat;
    long time_sleep;
    long time_think;

    if (!pre_simulation)
        write_status(THINKING, philo, DEBUG_MODE);

    // if philonr is even return
    if (philo->table->philo_nr % 2 == 0)
        return;

    // ODD nr
    time_eat   = philo->table->time_to_eat;
    time_sleep = philo->table->time_to_sleep;
    time_think = time_eat * 2 - time_sleep; // available time to think

    if (time_think < 0)
        time_think = 0;

    // precise control for thinking time
    precise_usleep(time_think * 0.42, philo->table);
}

/**
 * Ad hoc function for handling
 * a single philo
 */
void *lone_philo(void *arg)
{
    t_philo *philo;

    philo = (t_philo *)arg;
    wait_all_threads(philo->table);
    set_long(&philo->philo_mutex, &philo->last_meal_time, gettime(MILLISECOND));
    increase_long(&philo->table->table_mutex, &philo->table->threads_running_nr);
    write_status(TAKE_FIRST_FORK, philo, DEBUG_MODE);
    while (!simulation_finished(philo->table))
        usleep(200);
    return (NULL);
}

/**
 * eat routing
 *
 * 1) grab the forks : here first and second fork is handy
 *
 * 2) eat : write eat, update last meal, update meals counter, evt bool full
 * 3) release the forks
 */
static void eat(t_philo *philo)
{
    // 1)
    safe_mutex_handle(&philo->first_fork->fork, LOCK);
    write_status(TAKE_FIRST_FORK, philo, DEBUG_MODE);
    safe_mutex_handle(&philo->second_fork->fork, LOCK);
    write_status(TAKE_SECOND_FORK, philo, DEBUG_MODE);

    // 2)
    set_long(&philo->philo_mutex, &philo->last_meal_time, gettime(MILLISECOND));
    philo->meals_counter++;
    write_status(EATING, philo, DEBUG_MODE);
    precise_usleep(philo->table->time_to_eat, philo->table);
    if (philo->table->nr_limit_meals > 0 && philo->meals_counter == philo->table->nr_limit_meals)
        set_bool(&philo->philo_mutex, &philo->full, true);

    // 3)
    safe_mutex_handle(&philo->first_fork->fork, UNLOCK);
    safe_mutex_handle(&philo->second_fork->fork, UNLOCK);
}

/**
 * 0) wait all philos, sync start
 * 1) endless loops
 */
void *dinner_simulation(void *data)
{
    t_philo *philo;
    philo = (t_philo *)data;

    // spinlock
    wait_all_threads(philo->table);

    // set time last meal
    set_long(&philo->philo_mutex, &philo->last_meal_time, gettime(MILLISECOND));

    // synch with monitor
    // inc a table variable counter with all the threads running
    increase_long(&philo->table->table_mutex, &philo->table->threads_running_nr);

    // desync philos
    desync_philos(philo);

    while (!simulation_finished(philo->table))
    {
        // 1) philo full
        if (philo->full)
            break;

        // 2) eat
        eat(philo);

        // 3) sleep -> write_status & precise usleep
        write_status(SLEEPING, philo, DEBUG_MODE);
        precise_usleep(philo->table->time_to_sleep, philo->table);
        // 4) think
        thinking(philo, false);
    }
    return (NULL);
}

/**
 * ./philo 5 800 200 200 [5]
 *
 * 0) if no meals, return ->[0]
 * 1) if only one philo -> ad hoc func
 * 2) create all threads, all philos
 * 3) create a monitor thread
 * 4) sync the beginning of the sim
 *          pthread_create -> philo starts running
 *          every philo start simultaneously
 * 5) JOIN everyone
 */
void dinner_start(t_table *table)
{
    int i;
    i = -1;

    if (0 == table->nr_limit_meals)
        return;
    else if (1 == table->philo_nr)
        safe_thread_handle(&table->philos[0].thread_id, lone_philo, &table->philos[0], CREATE);
    else
    {
        while (++i < table->philo_nr)
            safe_thread_handle(&table->philos[i].thread_id, dinner_simulation, &table->philos[i], CREATE);
    }

    // monitor
    safe_thread_handle(&table->monitor, monitor_dinner, table, CREATE);

    // start of simulation
    table->start_simulation = gettime(MILLISECOND);

    // all threads are ready
    set_bool(&table->table_mutex, &table->all_threads_ready, true);

    // wait for everyone
    i = -1;
    while (++i < table->philo_nr)
        safe_thread_handle(&table->philos[i].thread_id, NULL, NULL, JOIN);

    // if this line is reached, all philos are full
    set_bool(&table->table_mutex, &table->end_simulation, true);

    safe_thread_handle(&table->monitor, NULL, NULL, JOIN);
}
