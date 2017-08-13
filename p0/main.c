// Arquivo para testes durante desenvolvimento
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define N 100

typedef struct filaint_t
{
   struct filaint_t *prev ;  // ptr para usar cast com queue_t
   struct filaint_t *next ;  // ptr para usar cast com queue_t
   int id ;
   // outros campos podem ser acrescidos aqui
} filaint_t ;

filaint_t item[N];
filaint_t *fila0, *fila1, *aux, *final ;
int i;

int main (int argc, char **argv, char **envp)
{
	// inicializa os N elementos
	for (i = 0; i < N; i++)
	{
	   item[i].id = i;
	   item[i].prev = NULL;
	   item[i].next = NULL;
	}
	queue_append ((queue_t **) &fila0, (queue_t*) &item[0]);
	printf("primeiro %d\n", fila0->id);
	return 0;
}