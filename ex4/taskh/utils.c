#include "philo.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * get time of day
 *
 * time_code -> SECONDS MILLISECONDS MICROSECONDS
 */
long gettime(t_time_code time_code)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL))
        error_exit("Gettimeofday failed");
    if (SECOND == time_code)
        return (tv.tv_sec + (tv.tv_usec / 1e6));
    else if (MILLISECOND == time_code)
        return ((tv.tv_sec * 1e3) + (tv.tv_usec / 1e3));
    else if (MICROSECOND == time_code)
        return ((tv.tv_sec * 1e6) + tv.tv_usec);
    else
        error_exit("Wrong input to gettime.");
    return (1337);
}

/**
 * precise usleep
 *
 * is the sim finished?
 * 1) usleep the majority of the time, not CPU intensive
 * 2) refine last microsec with spinlock
 */
void precise_usleep(long usec, t_table *table)
{
    long start;
    long elapsed;
    long remaining;

    start = gettime(MICROSECOND);
    while (gettime(MICROSECOND) - start < usec)
    {
        // 1)
        if (simulation_finished(table))
            break;
        elapsed   = gettime(MICROSECOND) - start;
        remaining = usec - elapsed;

        // to get a spinlock threshold
        if (remaining > 1e3)
            usleep(remaining / 2);
        else
        {
            // spinlock
            while (gettime(MICROSECOND) - start < usec)
                ;
        }
    }
}

void clean(t_table *table)
{
    t_philo *philo;
    int      i;

    i = -1;
    while (++i < table->philo_nr)
    {
        philo = table->philos + i;
        safe_mutex_handle(&philo->philo_mutex, DESTROY);
    }
    safe_mutex_handle(&table->write_mutex, DESTROY);
    safe_mutex_handle(&table->table_mutex, DESTROY);
    free(table->forks);
    free(table->philos);
}

void error_exit(const char *error)
{
    printf(RED "%s \n" RST, error);
    exit(EXIT_FAILURE);
}
