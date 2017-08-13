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
	// verifica se a fila possui apenas 1 elemento
	if (queue->next == queue->prev) 
		return 1;
	aux = queue;
	while (aux->next) {
		cont++;
		aux = aux->next;
	}
	return cont;
}

void queue_append (queue_t **queue, queue_t *elem)
{
	queue_t *first, *aux;
	int q_size, i = 2;

	if (!queue || !elem) {
		printf("queue_append: fila ou elemento nao existe\n");
	}
	q_size = queue_size(*queue);
	first = aux = *queue;
	// se a fila esta vazia
	if (q_size <= 0) {
		(*queue) = elem;
		(*queue)->next = (*queue)->prev = elem;
		printf("insere na fila vazia\n");
	//se a fila possui apenas 1 elemento
	} else if (q_size == 1) {
		(*queue)->next = (*queue)->prev = elem;
		elem->next = elem->prev = (*queue);
		printf("insere primeiro elemento\n");
	} 
	else {
		aux = aux->next
		while (i <= q_size) {
			aux = aux->next;
		}
	}
}