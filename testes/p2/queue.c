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
	if (!elem || !queue) {
		printf("queue_append: parametros invalidos\n");
		return;
	}
	// elem já está em outra fila
	if (elem->next != NULL || elem->prev != NULL) {
		printf("queue_append: elem pertencente a outra fila\n");
		return;
	}
	// fila vazia
	if (!(*queue)) {
		(*queue) = elem;
		elem->next = elem->prev = elem;
	} else {
		aux = *queue;
		while (aux->next != *queue) {
			aux = aux->next;
		}
		aux->next = (*queue)->prev = elem;
		elem->next = *queue;
		elem->prev = aux;
	}
}

queue_t *queue_remove (queue_t **queue, queue_t *elem)
{
	queue_t *aux, *first;
	// parametros invalidos
	if (!queue || !elem) {
		printf("queue_remove: parametros invalidos\n");
		return NULL;
	}
	// fila vazia
	if (!(*queue)) {
		printf("queue_remove: fila vazia\n");
		return NULL;
	}
	aux = first = *queue;
	//fila com 1 elem
	if (aux->next == aux && aux->prev == aux && aux == elem) {
		(*queue) = elem->next = elem->prev = NULL;
		return elem;
	//elem eh o primeiro da fila	
	} else if (aux == elem) {
		(*queue) = elem->next;
	}
	while (aux != elem) {
		aux = aux->next;
		//elem nao está na fila
		if (aux == first) {
			printf("queue_remove: elem nao encontrado na fila\n");
			return NULL;
		}
	}
	//remove elem
	aux->prev->next = elem->next;
	aux->next->prev = elem->prev;
	elem->next = aux->next = NULL;
	elem->prev = aux->prev = NULL;
	return elem;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
	queue_t *aux = queue;
	if (!queue)
		printf("%s: []\n", name);
	else {
		printf("%s: [", name);
		print_elem(aux);
		while (aux->next != queue) {
			printf(" ");
			print_elem(aux->next);
			aux = aux->next;
		}
		printf("]\n");
	}
}