// GRR20182659 João Pedro Picolo

#include <stdlib.h>

#include "ppos.h"

// Global variables and definitions ===============================================
// Thread Stack's size
#define STACKSIZE 64*1024

task_t mainTask;
task_t *currentTask;
long long lastID;

// General functions ==============================================================
void ppos_init() {
    // Disables stdout buffer, used by printf()
    setvbuf(stdout, 0, _IONBF, 0);

    // Initializes mainTask
    mainTask.id = 0;                           // Main by default has id = 0
    getcontext(&(mainTask.context));           // Saves current context

    // Sets main as current context
    currentTask = &mainTask;

    // Updates lastID
    lastID = 0;

    #ifdef DEBUG
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
    makecontext(&(task->context), (void*)start_func, 1, arg);


    // Asigns the task a id
    lastID++;
    task->id = lastID;

    #ifdef DEBUG
    printf("PPOS: created task %d\n", task->id);
    #endif

    return task->id;
}			


void task_exit(int exitCode) {
    #ifdef DEBUG
    printf("PPOS: switch task %d to main task %d\n", currentTask->id, mainTask.id);
    #endif

    task_t temp = *currentTask;
    currentTask = &mainTask;

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