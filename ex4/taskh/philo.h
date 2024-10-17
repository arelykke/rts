#include <errno.h>
#include <limits.h>  //INT_MAX
#include <pthread.h> //mutex: init, destroy, lock, unlock | thread: createm join, detach
#include <stdbool.h>
#include <stdio.h>    //printf
#include <stdlib.h>   //malloc, free
#include <sys/time.h> //gettime
#include <unistd.h>   //wirte, usleep

// write function macro
#define DEBUG_MODE 1

/**
 * ANSI Escape Sequence for Bold Text Colors
 * Usage:
 *      printf(RED "This is red text." RST);
 *      printf(BLUE "This is blue text." RST);
 * Remember to use RST to reset the color after setting it.
 */
#define RST     "\033[0m"    // Reset to default color
#define RED     "\033[1;31m" // Bold Red
#define GREEN   "\033[1;32m" // Bold Green
#define YELLOW  "\033[1;33m" // Bold Yellow
#define BLUE    "\033[1;34m" // Bold Blue
#define MAGENTA "\033[1;35m" // Bold Magenta
#define CYAN    "\033[1;36m" // Bold Cyan
#define WHITE   "\033[1;37m" // Bold White

/**
 * PHILO STATES
 */
typedef enum e_status
{
    EATING,
    SLEEPING,
    THINKING,
    TAKE_FIRST_FORK,
    TAKE_SECOND_FORK,
    DIED,
} t_philo_status;

/**
 * OPCODE for mutex | thread functions
 */
typedef enum e_opcode
{
    LOCK,
    UNLOCK,
    INIT,
    DESTROY,
    CREATE,
    JOIN,
    DETACH,
} t_opcode;

/**
 * CODE for gettime
 */
typedef enum e_time_code
{
    SECOND,
    MILLISECOND,
    MICROSECOND
} t_time_code;

//*** structures **
typedef pthread_mutex_t t_mtx;
typedef struct s_table  t_table;

/**
 * FORK
 */
typedef struct s_fork
{
    t_mtx fork;
    int   fork_id;
} t_fork;

/**
 * PHILO
 *
 * -/philo 5 800 200 200 [5]
 */
typedef struct s_philo
{
    int       id;
    long      meals_counter;
    bool      full;
    long      last_meal_time; // time passed since last meal
    t_fork   *first_fork;
    t_fork   *second_fork;
    pthread_t thread_id;   // a philo is a thread
    t_mtx     philo_mutex; // usefull for races with the monitor
    t_table  *table;
} t_philo;

/**
 * TABLE
 */
typedef struct s_table
{
    long      philo_nr;
    long      time_to_die;
    long      time_to_eat;
    long      time_to_sleep;
    long      nr_limit_meals; // [5] | FLAG if -1
    long      start_simulation;
    long      threads_running_nr;
    bool      end_simulation;    // philo dies or all philos full
    bool      all_threads_ready; // sync philos
    pthread_t monitor;           //
    t_mtx     table_mutex;       // avoid race when reading from table
    t_mtx     write_mutex;
    t_fork   *forks;  // array forks
    t_philo  *philos; // array of philos
} t_table;

// *** PROTOTYPES ***

// ** monitoring **
void *monitor_dinner(void *data);

// ** odd number of philos **
void thinking(t_philo *philo, bool pre_simulation);

// ** dinner **
void *dinner_simulation(void *data);
void  dinner_start(t_table *table);

// ** init **
void data_init(t_table *table);

// ** write **
void write_status(t_philo_status status, t_philo *philo, bool debug);

// ** utils **
void error_exit(const char *error);
long gettime(t_time_code time_code);
void precise_usleep(long usec, t_table *table);
void clean(t_table *table);

// ** parsing **
void parse_input(t_table *table, char **av);

// ** safe functions **
void *safe_malloc(size_t bytes);
void  safe_thread_handle(pthread_t *thread, void *(*func)(void *), void *data, t_opcode opcode);
void  safe_mutex_handle(t_mtx *mutex, t_opcode opcode);

// ** sync utils **
void wait_all_threads(t_table *table);
bool all_threads_running(t_mtx *mutex, long *threads, long philo_nr);
void increase_long(t_mtx *mutex, long *value);
void desync_philos(t_philo *philo);

// ** setters and getters **
void set_bool(t_mtx *mutex, bool *dest, bool value);
bool get_bool(t_mtx *mutex, bool *value);
void set_long(t_mtx *mutex, long *dest, long value);
long get_long(t_mtx *mutex, long *value);
bool simulation_finished(t_table *table);
