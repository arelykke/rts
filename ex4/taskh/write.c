#include "philo.h"

/**
 * same as write, just with more info
 * helping when debugging
 */
static void write_status_debug(t_philo_status status, t_philo *philo, long elapsed)
{
    if (TAKE_FIRST_FORK == status && !simulation_finished(philo->table))
        printf(WHITE "%6ld" RST " %d has taken the 1. fork."
                     "\t\t\t" BLUE "[%d]fork-ID\n" RST,
               elapsed, philo->id, philo->first_fork->fork_id);
    else if (TAKE_SECOND_FORK == status && !simulation_finished(philo->table))
        printf(WHITE "%6ld" RST " %d has taken the 2. fork."
                     "\t\t\t" BLUE "[%d]fork-ID\n" RST,
               elapsed, philo->id, philo->second_fork->fork_id);
    else if (EATING == status && !simulation_finished(philo->table))
        printf(WHITE "%6ld" RST " %d is eating."
                     "\t\t\t\t" YELLOW "[%ld]meals\n" RST,
               elapsed, philo->id, philo->meals_counter);
    else if (SLEEPING == status && !simulation_finished(philo->table))
        printf(WHITE "%6ld" RST " %d is sleeping.\n", elapsed, philo->id);
    else if (THINKING == status && !simulation_finished(philo->table))
        printf(WHITE "%6ld" RST " %d is thinking.\n", elapsed, philo->id);
    else if (DIED == status)
        printf(RED "\t\t %6ld %d died.\n" RST, elapsed, philo->id);
}

/**
 * Function to write the philo status
 * in a thread safe manner
 * write
 * philos mutex to read meals counter
 * talbes mutex to read if end_simulation
 *
 * thread safe
 */
void write_status(t_philo_status status, t_philo *philo, bool debug)
{
    long elapsed;

    elapsed = gettime(MILLISECOND) - philo->table->start_simulation;

    if (philo->full)
        return;

    safe_mutex_handle(&philo->table->write_mutex, LOCK);

    if (debug)
        write_status_debug(status, philo, elapsed);
    else
    {
        if ((TAKE_FIRST_FORK == status || TAKE_SECOND_FORK == status) && !simulation_finished(philo->table))
            printf(WHITE "%-6ld" RST " %d has taken a fork\n", elapsed, philo->id);
        else if (EATING == status && !simulation_finished(philo->table))
            printf(WHITE "%-6ld" RST " %d is eating.\n", elapsed, philo->id);
        else if (SLEEPING == status && !simulation_finished(philo->table))
            printf(WHITE "%-6ld" RST " %d is sleeping.\n", elapsed, philo->id);
        else if (THINKING == status && !simulation_finished(philo->table))
            printf(WHITE "%-6ld" RST " %d is thinking.\n", elapsed, philo->id);
        else if (DIED == status)
            printf(RED "%-6ld" RST " %d has died.\n", elapsed, philo->id);
    }

    safe_mutex_handle(&philo->table->write_mutex, UNLOCK);
}
