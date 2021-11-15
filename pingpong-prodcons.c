// GRR20182659 João Pedro Picolo

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define MAX_ITEM 100
#define MAX_BUFFER 5

typedef struct filaint_t
{
   struct filaint_t *prev;
   struct filaint_t *next;
   int id;
} filaint_t ;


int item;

filaint_t *buffer = NULL;
task_t p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga;

void produtor(void *arg) {
    while (1) {
        task_sleep(1000);
        item = rand() % MAX_ITEM;

        sem_down(&s_vaga);

        sem_down(&s_buffer);
        if (queue_size((queue_t*) buffer) < MAX_BUFFER) {
            printf("%s %d\n", (char *)arg, item);

            filaint_t new;
            new.prev = NULL;
            new.next = NULL;
            new.id = item;
            queue_append((queue_t**) &buffer, (queue_t*)&new);
        }
        sem_up(&s_buffer);

        sem_up(&s_item);
    }
}

void consumidor(void *arg) {
    while (1) {
        sem_down(&s_item);

        sem_down(&s_buffer);
        if (queue_size((queue_t*) buffer) > 0) {
            printf("%s %d\n", (char *)arg, buffer->id);
            queue_remove((queue_t**) &buffer, (queue_t*)buffer);
        }
        sem_up(&s_buffer);

        sem_up(&s_vaga);
        task_sleep(1000);
    }
}

int main(int argc, char *argv[]) {
    // Starts main
    // printf("Main: begin\n");
    ppos_init();

    // Initializes semaphores
    sem_create(&s_buffer, 1);
    sem_create(&s_item, 1);

    // TODO - Should be total of slots, not 1
    sem_create(&s_vaga, 1);

    // Create tasks tarefas
    task_create(&p1, produtor, "p1 produziu ");
    task_create(&p2, produtor, "p2 produziu ");
    task_create(&p3, produtor, "p3 produziu ");
    task_create(&c1, consumidor, "\t\t\tc1 consumiu ");
    task_create(&c2, consumidor, "\t\t\tc2 consumiu ");

    // Main only ends after c2
    task_join(&c2);

    // Destroys semaphores
    sem_destroy(&s_buffer);
    sem_destroy(&s_item);
    sem_destroy(&s_vaga);

    // Exits main
    task_exit(0);
    exit(0);
}