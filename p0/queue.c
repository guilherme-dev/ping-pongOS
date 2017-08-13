// PingPongOS - PingPong Operating System
// P0: Biblioteca de filas
// Guilherme Gomes dos Santos GRR20124499
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int queue_size (queue_t *queue)
{
	int cont = 0;
	queue_t *aux;
	// verifica se a fila existe
	if (!queue)
		return 0;
	aux = queue;
	do {
		cont++;
		aux = aux->next;
	} while (aux != queue);
	return cont;
}

void queue_append (queue_t **queue, queue_t *elem)
{
	queue_t *aux;
	// parametros invalidos
	if(!elem || !queue){
		printf("erro1\n");
	}
	// fila vazia
	if (!(*queue)) {
		(*queue) = elem;
		elem->next = elem->prev = elem;
	} else {
		aux = *queue;
		while(aux->next != *queue){
			aux = aux->next;
		}
		aux->next = (*queue)->prev = elem;
		elem->next = *queue;
		elem->prev = aux;
	}
}

queue_t *queue_remove (queue_t **queue, queue_t *elem){
	return elem;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){

}