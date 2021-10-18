// GRR20182659 João Pedro Picolo

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include "ppos.h"

// Global variables and definitions ===============================================
// Thread Stack's size
#define STACKSIZE 64*1024

task_t mainTask, dispatcherTask;
task_t *currentTask;

task_t *readyQueue = NULL;

long long lastID, userTasks;

// Used for task preemption
struct sigaction action;
struct itimerval timer;

// General functions ==============================================================
task_t* scheduler() {
    task_t *highestTask = readyQueue;

    // Checks for highest priority (-20 is highest and 20 is lowest)
    task_t *temp = readyQueue;
    do {
        if(temp->dynamicPriority <= highestTask->dynamicPriority)  {
            highestTask = temp;
        }

        // Aging factor is -1, limited to -20
        if(temp->dynamicPriority > -20) {
            temp->dynamicPriority--;
        }
        temp = temp->next;
    } while(temp != readyQueue);

    #ifdef DEBUG
    printf("PPOS: Scheduler picking task with id %d and priority %d\n", highestTask->id, highestTask->dynamicPriority);
    #endif

    highestTask->dynamicPriority = highestTask->staticPriority;
    return highestTask;
}

void taskDispatcher() {
    task_t *nextTask = NULL;

    while(userTasks > 0) {
        nextTask = scheduler();

        if(nextTask != NULL) {
            task_switch(nextTask);

            switch(nextTask->status) {
                case READY:
                    #ifdef DEBUG
                    printf("PPOS: task %d with status READY\n", nextTask->id);
                    #endif
                    break;
                case COMPLETE:
                    free(nextTask->context.uc_stack.ss_sp);
                    task_t *prevTask = nextTask->prev;
                    queue_remove((queue_t**)&readyQueue, (queue_t*)nextTask);
                    nextTask = prevTask;
                    #ifdef DEBUG
                    printf("PPOS: task %d with status COMPLETE. Cleaned its stack.\n", nextTask->id);
                    #endif
                    break;
                case SUSPENDED:
                    #ifdef DEBUG
                    printf("PPOS: task %d with status SUSPENDED\n", nextTask->id);
                    #endif
                    break;
                default:
                    #ifdef DEBUG
                    printf("PPOS: task %d with status unknown\n", nextTask->id);
                    #endif
                    break;
            }
        }
    }

    task_exit(0);
}

void tickHandler() {
    if (!(currentTask->systemTask)) {
        currentTask->quantum--;

        if(currentTask->quantum == 0) {
            #ifdef DEBUG
            printf("PPOS: task %d quantum reached zero\n", currentTask->id);
            #endif
            task_yield();
        }
    }
}

void initializeTicker() {
    action.sa_handler = tickHandler;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction(SIGALRM, &action, 0) < 0) {
        perror("Sigaction error: ");
        exit(1);
    }

    // Adjusts timer value
    timer.it_value.tv_usec = 1000;      // First shot, in microseconds
    timer.it_value.tv_sec = 0;          // First shot, in seconds
    timer.it_interval.tv_usec = 1000;   // Following shoots, in microseconds
    timer.it_interval.tv_sec = 0;       // Following shots, in seconds

    // Sets timer interval to fire at each milisecond
    if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
        perror("Setitimer error: ");
        exit(1);
    }

    #ifdef DEBUG
    printf("PPOS: time action initialized\n");
    #endif
}

void ppos_init() {
    // Disables stdout buffer, used by printf()
    setvbuf(stdout, 0, _IONBF, 0);

    // Initialize user tasks
    userTasks = 0;

    // Updates lastID
    lastID = 0;

    // Initializes mainTask
    mainTask.id = lastID;                       // Main by default has id = 0
    mainTask.systemTask = 1;                    // Sets main as system task
    getcontext(&(mainTask.context));            // Saves current context

    // Sets main as current context
    currentTask = &mainTask;

    // Creates dispatcher task
    task_create(&dispatcherTask, &taskDispatcher, NULL);
    dispatcherTask.systemTask = 1;
    
    initializeTicker();

    #ifdef DEBUG
    printf("PPOS: created task dispatcher with id %d\n", dispatcherTask.id);
    printf("PPOS: system initialized\n");
    #endif
}


// Tasks managements ==============================================================
int task_create(task_t *task,			        // New task descriptor
                 void (*start_func)(void *),	// Task's body function
                 void *arg) {                   // Task's argument

    char *stack;

    // Saves current context in the task
    getcontext(&(task->context));

    // Creates task's stack
    stack = (char*)malloc(STACKSIZE);
    if (stack) {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
    }
    else {
        perror("Error on task's stack creation\n");
        return -1;
    }

    // Updates task's stored context so it will point to right function
    if(arg != NULL) {
        makecontext(&(task->context), (void*)start_func, 1, arg);
    }
    else {
        makecontext(&(task->context), (void*)start_func, 0, arg);
    }

    // Asigns the task a id
    lastID++;
    task->id = lastID;

    // Tasks are user tasks by default
    task->systemTask = 0;

    // Inserts task in queue
    // Dispatcher (task->id == 1) can't be inserted
    if(task->id > 1) {
        #ifdef DEBUG
        printf("PPOS: appending task %d to queue\n", task->id);
        #endif

        // Set tasks fields
        task->status = READY;
        task->staticPriority = 0;
        task->dynamicPriority = task->staticPriority;

        // Adds tasks to ready tasks queue
        queue_append((queue_t **)&readyQueue, (queue_t*)task);

        // Increments user active tasks
        userTasks++;
    }

    #ifdef DEBUG
    printf("PPOS: created task %d\n", task->id);
    #endif

    return task->id;
}			


void task_exit(int exitCode) {
    task_t temp = *currentTask;

    // If exits from dispatcher, goes to main
    if(currentTask == &dispatcherTask) {
        currentTask = &mainTask;
    }
    else {
        // Decrement user active tasks
        userTasks--;

        // Changes task status to complete
        currentTask->status = COMPLETE;

        currentTask = &dispatcherTask;
    }

    #ifdef DEBUG
    printf("PPOS: switch task %d to task %d\n", temp.id, currentTask->id);
    #endif

    // Changes the current context to main
    swapcontext(&(temp.context), &(currentTask->context));
}


int task_switch(task_t *task) {
    // Checks is task exists
    if(task == NULL) {
        perror("Failed to switch context, task not found\n");
        return -1;
    }

    #ifdef DEBUG
    printf("PPOS: switch task %d to task %d\n", currentTask->id, task->id);
    #endif

    task_t *temp = currentTask;
    currentTask = task;
    if (!(currentTask->systemTask)) {
        // Each task has a quantum of 20 each time it gets the processor
        currentTask->quantum = 20;
    }

    // Saves current context on memory pointed by first parameter
    // then restores to the context saved in the second parameter
    swapcontext(&(temp->context), &(task->context));

    return 0;
}

int task_id() {
    return currentTask->id;
}


// Scaling Functions ==============================================================
void task_yield() {
    #ifdef DEBUG
    printf("PPOS: changing context to dispatcher\n");
    #endif

    task_switch(&dispatcherTask);
}

void task_setprio(task_t *task, int prio) {
    if(prio > 20 || prio < -20) {
        perror("Invalid priority. Value found: %d, expected value be between -20 and 20.\n");
    }

    if(task == NULL) {
        currentTask->staticPriority = prio;
        currentTask->dynamicPriority = currentTask->staticPriority;

        #ifdef DEBUG
        printf("PPOS: Seeting task with id %d priority to %d\n", currentTask->id, prio);
        #endif
    }
    else {
        task->staticPriority = prio;
        task->dynamicPriority = task->staticPriority;
    
        #ifdef DEBUG
        printf("PPOS: Seeting task with id %d priority to %d\n", task->id, prio);
        #endif
    }
}

int task_getprio (task_t *task) {
    if(task == NULL) {
        #ifdef DEBUG
        printf("PPOS: Task with id %d priority is %d\n", currentTask->id, currentTask->staticPriority);
        #endif

        return currentTask->staticPriority;
    }
    else {
        #ifdef DEBUG
        printf("PPOS: Task with id %d priority is %d\n", task->id, task->staticPriority);
        #endif

        return task->staticPriority;
    }
}