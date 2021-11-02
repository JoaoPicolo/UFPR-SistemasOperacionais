// GRR20182659 João Pedro Picolo

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// POSIX context change library
#include "queue.h"		    // Generic queue library

typedef enum task_status_t {
  READY,
  COMPLETED,
  SUSPENDED,
  SLEEPING,
} task_status_t;

// Task Control Block (TCB) Struct Definition
typedef struct task_t {
  struct task_t *prev, *next;		          // Pointers to be used in queue
  int id;				                          // Task identifier
  ucontext_t context;			                // Task's stored context
  task_status_t status;                   // Task's system status
  int dynamicPriority, staticPriority;    // Task's priorities
  int systemTask;						  // Tells if task belongs to the system or to the user
  int quantum;							  // Task's quantom for processor use
  unsigned int startTime, processorTime, activations; // Helper variables to collect statistics about the task
  struct task_t *suspendedQueue;          // Task's waintg to be awaked
  int exitCode;                           // Task's exit code
  int awakeTime;                          // If task sleeps, when it should be awaked
} task_t;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
