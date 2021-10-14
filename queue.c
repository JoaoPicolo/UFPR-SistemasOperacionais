// GRR20182659 João Pedro Picolo

#include <stdio.h>
#include "queue.h"

int queue_size (queue_t *queue) {
    int size = 0;

    if(queue == NULL) {
        return size;
    }

    // Adds while element hasn't gone back to start
    queue_t *temp = queue;
    do {
        size++;
        temp = temp->next;
    } while(temp != queue);

    return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    printf("%s [", name);

    int size = queue_size(queue);
    if(size != 0) {
        // Prints n - 1 elements, only used for output formatting
        queue_t *temp = queue;
        int count = 0;
        do {
            print_elem(temp);
            printf(" ");

            temp = temp->next;
            count++;
        } while((temp != queue) && (count < size - 1));

        // Prints last element
        print_elem(temp);
    }

    printf("]\n");
}

int queue_append (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        fprintf(stderr, "### ERROR: Queue doesn't exist\n");
        return -1;
    }

    if(elem == NULL) {
        fprintf(stderr, "### ERROR: Element doesn't exist\n");
        return -1;
    }

    if(elem->prev != NULL || elem->next != NULL) {
        fprintf(stderr, "### ERROR: Element already belongs to a queue\n");
        return -1;
    }

    // Inserts element
    if(*queue == NULL) {
        *queue = elem;
        (*queue)->prev = *queue;
        (*queue)->next = *queue;
    }
    else {
        queue_t *lastElem = (*queue)->prev;
        
        // Takes care of "right" direction
        lastElem->next = elem;
        elem->next = *queue;

        // Taks car of "left" direction
        elem->prev = lastElem;
        (*queue)->prev = elem;
    }

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        fprintf(stderr, "### ERROR: Queue doesn't exist\n");
        return -1;
    }

    if(*queue == NULL) {
        fprintf(stderr, "### ERROR: Queue is empty\n");
    }

    if(elem == NULL) {
        fprintf(stderr, "### ERROR: Element doesn't exist\n");
        return -1;
    }

    if(elem->prev == NULL || elem->next == NULL) {
        fprintf(stderr, "### ERROR: Element doesn't belong to any queue\n");
        return -1;
    }

    // Check if element belongs to the queue
    queue_t *temp = *queue;
    int elemFound = 0;
    do {
        if(temp == elem) {
            // Corrects temp position error, caused by iteration
            temp = temp->prev;
            elemFound = 1;
        }
        temp = temp->next;
    } while(temp != *queue && !elemFound);

    if(!elemFound) {
        fprintf(stderr, "### ERROR: Element doesn't belong to the queue\n");
        return -1;
    }


    // Removes element from queue
    if(queue_size(*queue) == 1) {
        *queue = NULL;
    }
    else {
        // If is removing first element, updates pointer as well
        if(*queue == elem) {
            *queue = elem->next;
        }

        // Fix queue remaining pointers
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }

    // Fix element pointers
    elem->next = NULL;
    elem->prev = NULL;

    return 0;
}