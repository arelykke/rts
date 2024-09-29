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

                /*  
                int simulationTime = globalTime + timeToRun;
                printf("Sim time: %d\n", simulationTime);
                while (globalTime < simulationTime)
                {
                    printf("global time: %d\n", globalTime);
                    usleep(timeUnitUs);
                    pthread_mutex_lock(&timeMutex);
                    //globalTime++;
                    
                    pthread_mutex_unlock(&timeMutex);

                }
                pthread_mutex_lock(&timeMutex);             
                currentTask->currentRuntime += timeToRun;
                pthread_mutex_unlock(&timeMutex);



                printf("\nruntime: +%d\n\n", timeToRun);
                additionalTime = timeToRun;
                */
                
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
    getchar();


}