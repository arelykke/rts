#include "philo.h"

/**
 * ./philo  5           800         200         200             [5]
 *          n_phlios    t_to_die    t_to_eat    t_to_sleep      n_meals (optional)
 */
int main(int ac, char **av)
{
    t_table table;

    if (5 == ac || 6 == ac)
    {
        // correct input
        // 1)
        parse_input(&table, av); // errorchecking, filling table

        // 2)
        data_init(&table);

        // 3)
        dinner_start(&table);

        // 4)
        clean(&table); // no leaks -> philos full | philos dies
    }
    else
    {
        // invalid input
        error_exit("Wrong input:\n" GREEN "Correct is <./philo 5 800 200 200 [5]>" RST);
    }

    return 0;
}
