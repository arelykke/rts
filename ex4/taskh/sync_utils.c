#include "philo.h"

void wait_all_threads(t_table *table)
{
    while (!get_bool(&table->table_mutex, &table->all_threads_ready))
        ;
}

/**
 * monitor busy waits until
 * all threads are not running
 */
bool all_threads_running(t_mtx *mutex, long *threads, long philo_nr)
{
    bool ret;

    ret = false;
    safe_mutex_handle(mutex, LOCK);
    if (*threads == philo_nr)
        ret = true;
    safe_mutex_handle(mutex, UNLOCK);
    return (ret);
}

/**
 * Increase threads running
 * to sync with the monitor
 */
void increase_long(t_mtx *mutex, long *value)
{
    safe_mutex_handle(mutex, LOCK);
    (*value)++;
    safe_mutex_handle(mutex, UNLOCK);
}

/**
 * makes system fair for odd nr of philos
 */
void desync_philos(t_philo *philo)
{
    if (philo->table->philo_nr % 2 == 0)
    {
        if (philo->id % 2 == 0)
            precise_usleep(3e4, philo->table);
    }
    else
    {
        if (philo->id % 2)
        {
            thinking(philo, true);
        }
    }
}
