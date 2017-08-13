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

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_elem (void *ptr)
{
   filaint_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

int main (int argc, char **argv, char **envp)
{
	// inicializa os N elementos
	for (i = 0; i < N; i++)
	{
	   item[i].id = i;
	   item[i].prev = NULL;
	   item[i].next = NULL;
	}
	fila0 = NULL ;
	// for (i=0; i<N; i++)
	// {
	   queue_append ((queue_t **) &fila0, (queue_t*) &item[0]) ;
	   queue_append ((queue_t **) &fila0, (queue_t*) &item[1]) ;
	   queue_append ((queue_t **) &fila0, (queue_t*) &item[2]) ;
	   queue_append ((queue_t **) &fila0, (queue_t*) &item[3]) ;
	   printf("%p\n", item[0].next); 
	   printf("%p\n", item[1].next); 
	   printf("%p\n", item[2].next); 
	   printf("%p\n", item[3].next); 
	// }
	return 0;
}