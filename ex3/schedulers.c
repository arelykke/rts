#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "scheduling.h"
#include "schedulers.h"
#include <limits.h>

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

// Implement your schedulers here!
void first_come_first_served(struct Task **tasks, int taskCount, int timeout)
{
    // Implement your solution here

    // sort by arrival time
    for(int i = 0; i < taskCount - 1; i++){
        for(int j = i + 1; j < taskCount; j++){
            if(tasks[i]->arrivalTime > tasks[j]->arrivalTime){
                struct Task *temp = tasks[j];
                tasks[i] = tasks[j];
                tasks[j] = temp;
            }
        }
    }

    for(int i = 0; i < taskCount; i++){
        struct Task *currentTask = tasks[i];

        //wait for task to arrive
        while(globalTime < currentTask->arrivalTime){
            usleep(timeUnitUs);
        }

        // set the task as running and track start time
        if(currentTask->startTime == -1){
            currentTask->startTime = globalTime;
        }

        set_task_state(currentTask, running);

        // run the task to completion (non-preemptive)
        while(currentTask->currentRuntime < currentTask->totalRuntime){
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            if (currentTask->currentRuntime < currentTask->totalRuntime)
            {
                currentTask->currentRuntime++;
            }
            pthread_mutex_unlock(&timeMutex);
        }

        set_task_state(currentTask, finished);

        if(globalTime >= timeout){
            break;
        }
    }
}
void shortest_process_next(struct Task **tasks, int taskCount, int timeout)
{
    // Implement your solution here
    int completedTasks = 0;
    while (completedTasks < taskCount && globalTime < timeout)
    {
            struct Task *shortestTask = NULL;

        for(int i = 0; i < taskCount; i++)
        {
            struct Task *task = tasks[i];

            if(task->arrivalTime <= globalTime && task->state != finished)
            {
                if(shortestTask == NULL || task->totalRuntime < shortestTask->totalRuntime)
                {
                    shortestTask = task;
                }
            }
        }

        if (shortestTask == NULL)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }

        if (shortestTask->startTime == -1)
        {
            shortestTask->startTime = globalTime;
        }
        set_task_state(shortestTask, running);

        while (shortestTask->currentRuntime < shortestTask->totalRuntime)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            if (shortestTask->currentRuntime < shortestTask->totalRuntime)
            {
                shortestTask->currentRuntime++;
            }
            pthread_mutex_unlock(&timeMutex);
        
        }

        set_task_state(shortestTask, finished);
        completedTasks++;
    }

}
void highest_response_ratio_next(struct Task **tasks, int taskCount, int timeout)
{
    // Implement your solution here

    int completedTasks = 0;

    while (completedTasks < taskCount && globalTime < timeout)
    {
        struct Task *highestRatioTask = NULL;
        double highestResponseRatio = -1.0;

        // sort task based on response ratio
        for (int i = 0; i < taskCount; i++)
        {
            struct Task *task = tasks[i];

            // task must have arrived but not finished yet
            if (task->arrivalTime <= globalTime && task->state != finished)
            {
                // calc wait time and response ratio
                int waitingTime = globalTime - task->arrivalTime;
                double responseRatio = (waitingTime + task->totalRuntime) / (double)task->totalRuntime;

                // check if this has the highest response ratio so far
                if (responseRatio > highestResponseRatio)
                {
                    highestResponseRatio = responseRatio;
                    highestRatioTask = task;
                }
            }
        }

        // if no task has arrived yet, wait for the next one
        if (highestRatioTask == NULL)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }

        // set the task as running and track start time
        if (highestRatioTask->startTime == -1)
        {
            highestRatioTask->startTime = globalTime;
        }

        set_task_state(highestRatioTask, running);

        // run the task to completion
        while (highestRatioTask->currentRuntime < highestRatioTask->totalRuntime)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            if (highestRatioTask->currentRuntime < highestRatioTask->totalRuntime)
            {
                highestRatioTask->currentRuntime++;
            }
            pthread_mutex_unlock(&timeMutex);
        }

        set_task_state(highestRatioTask, finished);
        completedTasks++;
    }
}
void shortest_remaining_time(struct Task **tasks, int taskCount, int timeout, int quantum)
{
    // Implement your solution here

    int completedTasks = 0;

    while (completedTasks < taskCount && globalTime < timeout)
    {
        struct Task *shortestTask = NULL;
        int shortestRemainingTime = INT_MAX;

        // find the task with the shortest remaining time that has arrived and is not finished
        for (int i = 0; i < taskCount; i++)
        {
            struct Task *task = tasks[i];

            // task must have arrived and not be finished yet
            if (task->arrivalTime <= globalTime && task->state != finished)
            {
                int remainingTime = task->totalRuntime - task->currentRuntime;

                // check if this task has the shortest remaining time so far
                if (remainingTime < shortestRemainingTime)
                {
                    shortestRemainingTime = remainingTime;
                    shortestTask = task;
                }
            }
        }

        // if no task has arrived yet, wait for the next one
        if (shortestTask == NULL)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
            continue;
        }

        // set the task as running and track start time
        if (shortestTask->startTime == -1)
        {
            shortestTask->startTime = globalTime;
        }

        set_task_state(shortestTask, running);

        // run the task for one time unit or until its finished
        usleep(timeUnitUs);
        pthread_mutex_lock(&timeMutex);
        globalTime++;
        if (shortestTask->currentRuntime < shortestTask->totalRuntime)
        {
            shortestTask->currentRuntime++;
        }
        pthread_mutex_unlock(&timeMutex);

        // check if task has finished
        if (shortestTask->currentRuntime >= shortestTask->totalRuntime)
        {
            set_task_state(shortestTask, finished);
            completedTasks++;
        }
    }
}

#define MAX_QUEUES 3 // define number of queues
#define MAX_QUEUE_SIZE 3

struct Queue {
    struct Task **tasks;
    int front;
    int rear;
    int count;
};

struct Queue queues[MAX_QUEUES];
int currentQueue = 0;

void initialize_queues(int taskCount)
{
    for (int i = 0; i < MAX_QUEUES; i++)
    {
        queues[i].tasks = malloc(MAX_QUEUE_SIZE * sizeof(struct Task *));
        if (queues[i].tasks == NULL)
        {
            fprintf(stderr, "Memory allocation failed for queue %d\n", i);
            exit(EXIT_FAILURE); // exit if malloc fails
        }
        queues[i].front = 0;
        queues[i].rear = 0;
        queues[i].count = 0;
    }
}

void enqueue(struct Queue *queue, struct Task *task, int taskCount)
{  
    if (queue->count < MAX_QUEUE_SIZE)
    {
        queue->tasks[queue->rear] = task;
        queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
        queue->count++;
        printf("Front: %d\n", queue->front);
        printf("Rear: %d\n", queue->rear);
        printf("Count: %d\n", queue->count);
    } else {
        fprintf(stderr, "Queue is full, cannot enqueue task.\n");
    }
}

struct Task *dequeue(struct Queue *queue, int taskCount)
{
    if (queue->count == 0) return NULL;
    struct Task *task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->count--;
    return task;
}

void feedback(struct Task **tasks, int taskCount, int timeout, int quantum)
{

    // init global time and task state management
    globalTime = 0;
    initialize_queues(taskCount);

    // enqueue all tasks into the highest priority queue initially
    for (int i = 0; i < taskCount; i++)
    {
        enqueue(&queues[0], tasks[i], taskCount);
    }

    while (globalTime < timeout)
    {
        bool taskScheduled = false;

        // check if the current queue has tasks
        for (int currentQueue = 0; currentQueue < MAX_QUEUES; currentQueue++)
        {
            struct Task *currentTask = dequeue(&queues[currentQueue], taskCount);
            if(currentTask == NULL) continue;

            // wait for task to arrive
            while (globalTime < currentTask->arrivalTime)
            {
                usleep(timeUnitUs);
                pthread_mutex_lock(&timeMutex);
                globalTime++;
                pthread_mutex_unlock(&timeMutex);
            }

            // set the task as running and track start time
            if (currentTask->startTime == -1)
            {
                currentTask->startTime = globalTime;
            }
            set_task_state(currentTask, running);

            // run the task for the defined quantum or until completion
            int timeSpent = 0;

            while (timeSpent < quantum && currentTask->currentRuntime < currentTask->totalRuntime)
            {
                usleep(timeUnitUs);
                pthread_mutex_lock(&timeMutex);
                globalTime++;
                if (currentTask->currentRuntime < currentTask->totalRuntime)
                {
                    currentTask->currentRuntime++;
                }
                timeSpent++;
                pthread_mutex_unlock(&timeMutex);
            }

            // check if the task has finished
            if (currentTask->currentRuntime >= currentTask->totalRuntime)
            {  
                set_task_state(currentTask, finished);
                
            } else {
                set_task_state(currentTask, preempted);
                // move the task to next lower-priority queue
                if(currentQueue + 1 < MAX_QUEUES)
                {
                    enqueue(&queues[currentQueue + 1], currentTask, taskCount);
                }
            }
            taskScheduled = true;
            break; // break to start scheduling the next task
        }

        // if no tasks where scheduled, increment global time to avoid busy-waiting
        if (!taskScheduled)
        {
            usleep(timeUnitUs);
            pthread_mutex_lock(&timeMutex);
            globalTime++;
            pthread_mutex_unlock(&timeMutex);
        }
    }

    // free the allocated memory for the queues
    for (int i = 0; i < MAX_QUEUES; i++)
    {
        free(queues[i].tasks);
    }
}