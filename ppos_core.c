// GRR20182659 João Pedro Picolo

#include <stdlib.h>

#include "ppos.h"

// Global variables and definitions ===============================================
// Thread Stack's size
#define STACKSIZE 64*1024

task_t mainTask, dispatcherTask;
task_t *currentTask;

task_t *readyTasksQueue = NULL, *nextTask = NULL;

long long lastID, userTasks;

void taskDispatcher();

// General functions ==============================================================
void ppos_init() {
    // Disables stdout buffer, used by printf()
    setvbuf(stdout, 0, _IONBF, 0);

    // Initialize user tasks
    userTasks = 0;

    // Updates lastID
    lastID = 0;

    // Initializes mainTask
    mainTask.id = lastID;                       // Main by default has id = 0
    getcontext(&(mainTask.context));           // Saves current context

    // Sets main as current context
    currentTask = &mainTask;

    // Creates dispatcher task
    task_create(&dispatcherTask, &taskDispatcher, NULL);
    

    #ifdef DEBUG
    printf("PPOS: created task dispatcher with id %d\n", dispatcherTask.id);
    printf("PPOS: system initialized\n");
    #endif
}


// Tasks managements ==============================================================
task_t* scheduler() {
    if(nextTask == NULL) {
       return readyTasksQueue;
    }
    
    return nextTask->next;
}

void taskDispatcher() {
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
                    queue_remove((queue_t**)&readyTasksQueue, (queue_t*)nextTask);
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

    // Inserts task in queue
    // Dispatcher (task->id == 1) can't be inserted
    if(task->id > 1) {
        #ifdef DEBUG
        printf("PPOS: appending task %d to queue\n", task->id);
        #endif

        task->status = READY;
        queue_append((queue_t **)&readyTasksQueue, (queue_t*)task);

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
    task_switch(&dispatcherTask);
}