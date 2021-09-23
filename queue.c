// GRR20182659 João Pedro Picolo

#include "queue.h"

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

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

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {

}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        fprintf(stderr, "Queue doesn't exist");
        return -1;
    }

    if(elem == NULL) {
        fprintf(stderr, "Element doesn't exist");
        return -1;
    }

    if(elem->prev != NULL || elem->next != NULL) {
        fprintf(stderr, "Element already belongs to another queue");
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

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        fprintf(stderr, "Queue doesn't exist");
        return -1;
    }

    if(*queue == NULL) {
        fprintf(stderr, "Queue is empty");
    }

    if(elem == NULL) {
        fprintf(stderr, "Element doesn't exist");
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
        fprintf(stderr, "Element doesn't belong to the queue.");
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