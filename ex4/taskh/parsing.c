#include "philo.h"

static inline bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static inline bool is_space(char c)
{
    return ((c >= 9 && c <= 13) || 32 == c);
}

/**
 * 1) check for negatives
 * 2) check if the numbers is legit
 * 3) check for INT_MAX
 */
static const char *valid_input(const char *str)
{
    int         len; // lenght of str
    const char *number;

    len = 0;

    // 1) skip spaces
    while (is_space(*str))
        ++str;
    if (*str == '+')
        ++str;
    else if (*str == '-')
        error_exit("Negative numbers are not allowed.");
    if (!is_digit(*str))
        error_exit("The input is not a correct digit.");
    number = str;

    while (is_digit(*str++))
        ++len;
    if (len > 10)
        error_exit("The value is too big, INT_MAX is the limit.");
    return (number);
}

static long ft_atol(const char *str)
{
    long num;
    num = 0;

    str = valid_input(str);
    while (is_digit(*str))
        num = (num * 10) + (*str++ - 48);
    if (num > INT_MAX)
        error_exit("The value is too big, INT_MAX is the limit.");
    return (num);
}

/**
 *  ./philo     5       800     200     200     [5]
 *              av[1]   av[2]   av[3]...
 * check
 * 1) actual numbers
 * 2) not > INT_MAX
 * 3) timestams > 60ms
 */

void parse_input(t_table *table, char **av)
{
    table->philo_nr      = ft_atol(av[1]);
    table->time_to_die   = ft_atol(av[2]) * 1e3;
    table->time_to_eat   = ft_atol(av[3]) * 1e3;
    table->time_to_sleep = ft_atol(av[4]) * 1e3;

    if (table->time_to_die < 6e4 || table->time_to_sleep < 6e4 || table->time_to_die < 6e4)
        error_exit("Use timestamps greater than 60ms");
    if (av[5])
        table->nr_limit_meals = ft_atol(av[5]);
    else
        table->nr_limit_meals = -1;
}
