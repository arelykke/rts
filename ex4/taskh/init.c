#include "philo.h"

/**
 * EVEN ODD fork assignment
 */

static void assign_forks(t_philo *philo, t_fork *forks, int philo_position)
{
    int philo_nr;
    philo_nr           = philo->table->philo_nr;
    philo->first_fork  = &forks[(philo_position + 1) % philo_nr];
    philo->second_fork = &forks[philo_position];
    if (philo->id % 2)
    {
        philo->first_fork  = &forks[philo_position];
        philo->second_fork = &forks[(philo_position + 1) % philo_nr];
    }
}

static void philo_init(t_table *table)
{
    int      i;
    t_philo *philo;

    i = -1;
    while (++i < table->philo_nr)
    {
        philo                = table->philos + i;
        philo->id            = i + 1;
        philo->full          = false;
        philo->meals_counter = 0;
        philo->table         = table;
        safe_mutex_handle(&philo->philo_mutex, INIT);

        // ad hoc
        assign_forks(philo, table->forks, i); // i = pos in table
    }
}

void data_init(t_table *table)
{
    int i;
    i = -1;

    table->end_simulation     = false;
    table->all_threads_ready  = false;
    table->threads_running_nr = 0;
    safe_mutex_handle(&table->table_mutex, INIT);
    safe_mutex_handle(&table->write_mutex, INIT);
    table->philos = safe_malloc(sizeof(t_philo) * table->philo_nr);
    table->forks  = safe_malloc(sizeof(t_fork) * table->philo_nr);

    while (++i < table->philo_nr)
    {
        safe_mutex_handle(&table->forks[i].fork, INIT);
        table->forks[i].fork_id = i;
    }
    philo_init(table);
}
