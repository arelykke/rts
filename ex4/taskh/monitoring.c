#include "philo.h"

static bool philo_died(t_philo *philo)
{
    long elapsed;
    long time_to_die;

    if (get_bool(&philo->philo_mutex, &philo->full))
        return (false);

    elapsed     = gettime(MILLISECOND) - get_long(&philo->philo_mutex, &philo->last_meal_time);
    time_to_die = philo->table->time_to_die / 1e3;

    if (elapsed > time_to_die)
        return (true);
    return (false);
}

void *monitor_dinner(void *data)
{
    int      i;
    t_table *table;

    table = (t_table *)data;

    // makes sure all philos are running
    // spinlock until all threads run
    while (!all_threads_running(&table->table_mutex, &table->threads_running_nr, table->philo_nr))
        ;

    while (!simulation_finished(table))
    {
        i = -1;
        while (++i < table->philo_nr && !simulation_finished(table))
        {
            if (philo_died(table->philos + i)) // TODO
            {
                set_bool(&table->table_mutex, &table->end_simulation, true);
                write_status(DIED, table->philos + i, DEBUG_MODE);
            }
        }
    }
    return (NULL);
}
