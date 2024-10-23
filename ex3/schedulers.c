#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "scheduling.h"
#include "schedulers.h"
#include <limits.h>

// for debugging
const char* getStateString(enum taskState state)
{
    switch (state)
    {
    case idle:
        return "idle";
    case running:
        return "running";
    case preempted:
        return "preempted";
    case finished:
        return "finished";
    default:
        return "unknown";
    }
}

void printTask2(struct Task* task)
{   
    printf("ID: %d    runtime: %d/%d    arrived: %d     started: %d   |   state: %s", 
    task->ID, task->currentRuntime, task->totalRuntime, task->arrivalTime, task->startTime, getStateString(task->state));
    printf("\n");
}
    

void set_task_state(struct Task *task, enum taskState taskNewState)
{
    pthread_mutex_lock(&taskStateMutex);
    task->state = taskNewState;
    pthread_mutex_unlock(&taskStateMutex);
}

void wait_for_rescheduling(int quantum, struct Task *task)
{
    int startTime;
    int waitTime;

    pthread_mutex_lock(&timeMutex);
    startTime = globalTime;
    pthread_mutex_unlock(&timeMutex);

    do
    {
        pthread_mutex_lock(&timeMutex);
        pthread_cond_wait(&timeCond, &timeMutex);
        waitTime = globalTime - startTime;
        pthread_mutex_unlock(&timeMutex);
    } while (task->state != finished && waitTime < quantum);

    usleep(timeUnitUs / 100);
}

void round_robin(struct Task **tasks, int taskCount, int timeout, int quantum)
{
    int taskIndex = 0;

    do
    {
        // Skip finished tasks or those that have not arrived yet
        if (tasks[taskIndex]->state == finished || tasks[taskIndex]->arrivalTime > globalTime)
        {
            taskIndex = (taskIndex + 1) % taskCount;
            continue;
        }

        // Set the task state to running
        if (tasks[taskIndex]->startTime == -1)
            tasks[taskIndex]->startTime = globalTime;
        set_task_state(tasks[taskIndex], running);


        // Wait for the quantum interval
        wait_for_rescheduling(quantum, tasks[taskIndex]);

        //  Check if the task is finished
        if (tasks[taskIndex]->state == finished)
        {
        }
        else
        {
            set_task_state(tasks[taskIndex], preempted);
        }

        // Find the next task to run
        taskIndex = (taskIndex + 1) % taskCount;

    } while (globalTime < timeout);
}


void first_come_first_served(struct Task **tasks, int taskCount, int timeout)
{
    printf("List of tasks:\n");
    for (int i = 0; i < taskCount; i++)
    {
        printTask2(tasks[i]);
    }
    printf("\n\n\nStart simulation\n");
    int finishedTasks = 0;

    while (finishedTasks < taskCount && globalTime < timeout)
    {
        // pointer for task to be executed
        struct Task *firstTask = NULL;

        for (int t = 0; t < taskCount; t++)
        {
            struct Task *task = tasks[t];
            if (task->arrivalTime <= globalTime && task->state != finished)
            {
                if(firstTask == NULL || task->arrivalTime < firstTask->arrivalTime)
                {
                    firstTask = task;

                }
            }
        }

        // no tasks arrived, inc time
        if (firstTask == NULL)
        {
            printf("%d          | no new tasks...\n", globalTime);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }
        else
        {
            printf("%d          | task arrived...       |   ", globalTime);
            printTask2(firstTask);
        }

        // run first task
        if (firstTask->startTime == -1)
        {
            firstTask->startTime = globalTime;
        }
        set_task_state(firstTask, running);
        printf("%d          | executing task...     |   ", globalTime);
        printTask2(firstTask);
        int runtime = firstTask->totalRuntime - firstTask->currentRuntime;
        wait_for_rescheduling(runtime, firstTask);
        printf("\n+%d runtime\n\n", runtime);
        set_task_state(firstTask, finished);
        printf("%d          | task finished...      |   ", globalTime);
        printTask2(firstTask);
        printf("\n\n");
        finishedTasks++;
        
    }
}

void shortest_process_next(struct Task **tasks, int taskCount, int timeout)
{
    printf("List of tasks:\n");
    for (int i = 0; i < taskCount; i++)
    {
        printTask2(tasks[i]);
    }
    printf("\n\n\nStart simulation\n");

    int finishedTasks = 0;

    struct Task *queue[taskCount];
    int qIndex = 0;

    while (finishedTasks < taskCount && globalTime < timeout)
    {
        struct Task *shortestTask = NULL;

        qIndex = 0;

        // tasks arriving
        for (int t = 0; t < taskCount; t++)
        {   
            struct Task *task = tasks[t];
            if (task->arrivalTime <= globalTime && task->state != finished)
            {   
                queue[qIndex++] = task;
            }            
        }

        // sorting task queue
        if (qIndex > 0)
        {
            shortestTask = queue[0];
            for (int t = 0; t < qIndex; t++)
            {
                if (queue[t]->totalRuntime < shortestTask->totalRuntime)
                {
                    shortestTask = queue[t];
                }
            }
        }
        

        // no tasks arrived, inc time
        if (shortestTask == NULL)
        {
            printf("%d          | no new tasks...\n", globalTime);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }
        else
        {
            printf("%d          | task arrived...       |   ", globalTime);
            printTask2(shortestTask);
        }

        // run shortest task
        if (shortestTask->startTime == -1)
        {
            shortestTask->startTime = globalTime;
        }
        set_task_state(shortestTask, running);
        printf("%d          | executing task...     |   ", globalTime);
        printTask2(shortestTask);
        int runtime = shortestTask->totalRuntime - shortestTask->currentRuntime;
        wait_for_rescheduling(runtime, shortestTask);

        printf("\n+%d runtime\n\n", runtime);

        set_task_state(shortestTask, finished);
        printf("%d          | task finished...      |   ", globalTime);
        printTask2(shortestTask);
        printf("\n\n");
        finishedTasks++;
    }

}

void highest_response_ratio_next(struct Task **tasks, int taskCount, int timeout)
{
    printf("List of tasks:\n");
    for (int i = 0; i < taskCount; i++)
    {
        printTask2(tasks[i]);
    }
    printf("\n\n\nStart simulation\n");

    int finishedTasks = 0;

    struct Task *queue[taskCount];
    int qIndex = 0;

    while (finishedTasks < taskCount && globalTime < timeout)
    {
        struct Task *selectedTask = NULL;
        qIndex = 0;

        // tasks arriving
        for (int t = 0; t < taskCount; t++)
        {
            struct Task *task = tasks[t];
            if (task->arrivalTime <= globalTime && task->state != finished)
            {
                queue[qIndex++] = task;
            }
        }

        // select task with lower response ratio
        if (qIndex > 0)
        {
            int maxRatioIndex = 0;
            float maxResponseRatio = (globalTime - queue[0]->arrivalTime + queue[0]->totalRuntime) / (float)(queue[0]->totalRuntime);

            printf("\nQueue: ");
            printf("\n______________________________________________________________________\n");
            for (int t = 0; t < qIndex; t++)
            {
                struct Task *task = queue[t];
                int waitingTime = globalTime - task->arrivalTime;
                float responseRatio = (float)(waitingTime + task->totalRuntime) / task->totalRuntime;
                printf("\nresponse ratios: %f           | ", responseRatio);
                printTask2(queue[t]);

                if (responseRatio > maxResponseRatio)
                {
                    maxResponseRatio = responseRatio;
                    maxRatioIndex = t;
                }
            }
            printf("\n______________________________________________________________________\n");
            printf("\nSelected response ratio: %f\n", maxResponseRatio);
            selectedTask = queue[maxRatioIndex];
            printf("\nSelected task with highest response ratio %f \n", maxResponseRatio);
            printTask2(selectedTask);
            printf("\n______________________________________________________________________\n");

        }

         // no tasks arrived, inc time
        if (selectedTask == NULL)
        {
            printf("%d          | no new tasks...\n", globalTime);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }
        else
        {
            printf("%d          | task arrived...       |   ", globalTime);
            printTask2(selectedTask);
        }

        // run selected task
        if (selectedTask->startTime == -1)
        {
            selectedTask->startTime = globalTime;
        }
        set_task_state(selectedTask, running);
        printf("%d          | executing task...     |   ", globalTime);
        printTask2(selectedTask);
        int runtime = selectedTask->totalRuntime - selectedTask->currentRuntime;
        wait_for_rescheduling(runtime, selectedTask);

        printf("\n+%d runtime\n\n", runtime);

        set_task_state(selectedTask, finished);
        printf("%d          | task finished...      |   ", globalTime);
        printTask2(selectedTask);
        printf("\n\n");
        finishedTasks++;

    }
}

void shortest_remaining_time(struct Task **tasks, int taskCount, int timeout, int quantum)
{
       printf("List of tasks:\n");
    for (int i = 0; i < taskCount; i++)
    {
        printTask2(tasks[i]);
    }
    printf("\n\n\nStart simulation\n");

    int finishedTasks = 0;

    struct Task *queue[taskCount];
    int qIndex = 0;

    while(finishedTasks < taskCount && globalTime < timeout)
    {
        struct Task *selectedTask = NULL;
        qIndex = 0;

        // tasks arriving
        for (int t = 0; t < taskCount; t++)
        {
            struct Task *task = tasks[t];
            if (task->arrivalTime <= globalTime && task->state != finished)
            {
                queue[qIndex++] = task;
            }
        }

        // sort task queue based on shortest remaining time
        if (qIndex > 0)
        {
            int shortestRemainingTime = (queue[0]->totalRuntime - queue[0]->currentRuntime);
            int srtIndex = 0;
            

            printf("\nQueue: ");
            printf("\n______________________________________________________________________\n");
            for (int t = 0; t < qIndex; t++)
            {
                struct Task *task = queue[t];
                int remainingTime = (task->totalRuntime - task->currentRuntime);
                printf("\nremaining time: %d          | ", remainingTime);
                printTask2(queue[t]);

                if (remainingTime < shortestRemainingTime)
                {
                    shortestRemainingTime = remainingTime;
                    srtIndex = t;
                }
            }
            printf("\n______________________________________________________________________\n");

            printf("\nSelected SRT: %d\n", shortestRemainingTime);
            selectedTask = queue[srtIndex];
            printf("\nSelected task with remaining time %d \n", shortestRemainingTime);
            printTask2(selectedTask);
            printf("\n______________________________________________________________________\n");
        }

        // no tasks arrived, inc time
        if (selectedTask == NULL)
        {
            printf("%d          | no new tasks...\n", globalTime);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }
        else
        {
            printf("%d          | task arrived...       |   ", globalTime);
            printTask2(selectedTask);
        }

        // execute slected task
        if (selectedTask->startTime == -1) selectedTask->startTime = globalTime;
        set_task_state(selectedTask, running);
        printf("%d          | executing task...     |   ", globalTime);
        printTask2(selectedTask);
        int runtime = (selectedTask->totalRuntime - selectedTask->currentRuntime < quantum) ? selectedTask->totalRuntime - selectedTask->currentRuntime : quantum;

        wait_for_rescheduling(runtime, selectedTask);
        printf("\n+%d runtime\n\n", runtime);

        // check if task is finished
        if (selectedTask->currentRuntime >= selectedTask->totalRuntime)
        {
            set_task_state(selectedTask, finished);
            printf("%d          | task finished...      |   ", globalTime);
            printTask2(selectedTask);
            printf("\n\n");
            finishedTasks++;
        }
        else
        {
            set_task_state(selectedTask, preempted);
            printf("%d          | task preemted...      |   ", globalTime);
            printTask2(selectedTask);
            printf("\n\n");
        }

    }

}


#define MAX_TASKS 10
#define MAX_QUEUES 3

struct Queue{
    struct Task* tasks[MAX_TASKS];
    int front;
    int rear;
    int count;
};

void initQueue(struct Queue* q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        q->tasks[i] = NULL;
    }
}

int isFull(struct Queue* q)
{
    return q->count == MAX_TASKS;
}

int isEmpty(struct Queue* q)
{
    return q->count == 0;
}

void enqueue(struct Queue* q, struct Task* task)
{
    if(isFull(q)){
        printf("Queue is full. Cannot enqueue task %d\n", task->ID);
        return;
    }
    pthread_mutex_lock(&timeMutex);
    q->rear = (q->rear + 1) % MAX_TASKS;
    q->tasks[q->rear] = task;
    q->count++;
    pthread_mutex_unlock(&timeMutex);
    printf("EQ: |Task %d --> Q%d|", task->ID, task->Qnumber);
}

struct Task* dequeue(struct Queue* q)
{
    if(isEmpty(q)){
        printf("Queue is empty\n");
        return NULL;
    }

    struct Task* task = q->tasks[q->front];
    q->tasks[q->front] = NULL;
    q->front = (q->front + 1) % MAX_TASKS;
    q->count--;
    
    printf("DQ: |Task %d <-- Q%d|", task->ID, task->Qnumber);
    
    
    return task;
}




// for debugging
void printTask(struct Task* task)
{   

    printf("    runtime: %d/%d  start: %d  arrived: %d   |   state: %s", 
    task->currentRuntime, task->totalRuntime, task->startTime, task->arrivalTime, getStateString(task->state));
    printf("\n");
    
}



// for debugging
void printQueue(struct Queue* queue, int q)
{
    printf("\n");
    printf("Queue %d | Slots (%d/%d) \n", q, queue->count, MAX_TASKS);
    
    if (queue->count != 0)
    {
        printf("__________________________\n\n");
        for(int i = 0; i < MAX_TASKS; i++)
        {   
            printf("%d ", i);
            if(queue->tasks[i] != NULL)
            {
                
                printf("    ID: %d  ", queue->tasks[i]->ID);
                printTask(queue->tasks[i]);
            } 
            else{printf("    empty\n");}
        }
        printf("__________________________\n\n");
    }
    
}



void feedback(struct Task **tasks, int taskCount, int timeout, int quantum)
{
    // init queues
    struct Queue queues[MAX_QUEUES];
    for(int i = 0; i < MAX_QUEUES; i++)
    {
        initQueue(&queues[i]);
    }

    int finished_tasks = 0;
    int additionalTime = 0;
    bool enqueued[taskCount];
    bool is_started[taskCount];
    memset(enqueued, 0, sizeof(enqueued));
    memset(is_started, 0, sizeof(is_started));

    // main loop
    while (finished_tasks < taskCount)
    {
        printf("\n\n");
        printf("Global time %d\n", globalTime);
        printf("---------------------------------------------------------------------------------------------------\n");
        

        // enqueue tasks that have arrived at current runtime to the highest prio queue
        for (int i = 0; i < taskCount; i++)
        {
            if (!enqueued[i] && tasks[i]->arrivalTime <= globalTime) {
                printf("New task arrived!\n");
                enqueue(&queues[0], tasks[i]);
                enqueued[i] = true;
                tasks[i]->Qnumber = 0;
                tasks[i]->startTime = -1;
                //tasks[i]->arrivalTime = globalTime;
                is_started[i] = false;
                printTask(tasks[i]);
            }
        }

        // process tasks in the queues, starting from the highest priority
        for (int q = 0; q < MAX_QUEUES; q++)
        {  
            if (!isEmpty(&queues[q])) 
            {
                //print queues
                for(int i = 0; i < MAX_QUEUES; i++)
                {
                    printQueue(&queues[i], i);
                }
                printf("\n\n");
                    
                
                printf("Dequeue and execute task. \n");
                struct Task* currentTask = dequeue(&queues[q]);
                if (currentTask == NULL){continue;}

                // check if not started --> give starttime
                if(!is_started[currentTask->ID])
                {
                    pthread_mutex_lock(&timeMutex);
                    currentTask->startTime = globalTime;
                    pthread_mutex_unlock(&timeMutex);
                    is_started[currentTask->ID] = true;
                }

                // calculate how long to run task
                int timeToRun = (currentTask->totalRuntime - currentTask->currentRuntime < quantum) ? currentTask->totalRuntime - currentTask->currentRuntime : quantum;

                // run task
                set_task_state(currentTask, running);
                wait_for_rescheduling(timeToRun, currentTask);
                printTask(currentTask);

                
                // check if task is finished
                if (currentTask->currentRuntime >= currentTask->totalRuntime)
                {
                    set_task_state(currentTask, finished);
                    finished_tasks++;
                    printf("Task %d finished.   ", currentTask->ID);
                    printTask(currentTask);
                }     
                else
                {   
                    printf("Task not finished, ");
                    set_task_state(currentTask, preempted);

                    // demote if task has used full quantum
                    if (timeToRun == quantum && currentTask->Qnumber < MAX_QUEUES - 1)
                    {
                        printf("demoted to lower prio queue.\n");
                        currentTask->Qnumber++;
                        enqueue(&queues[currentTask->Qnumber], currentTask);
                    }
                    else
                    {
                        printf("re-enqued.\n");
                        enqueue(&queues[currentTask->Qnumber], currentTask);
                    }
                    
                    printTask(currentTask);
                    
                }
                break;
            }
        }
        printf("\n---------------------------------------------------------------------------------------------------\n");
     
    }
    
    for (int i = 0; i < MAX_QUEUES; i++)
    {
        printQueue(&queues[i], i);
    }
}
