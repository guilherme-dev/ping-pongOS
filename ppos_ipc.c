// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// Operações de IPC
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

// operações de IPC ============================================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value)
{
	if (!s)
		return -1;
	s->queue = NULL;
	s->counter = value;
	return 0;
}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
	return 0;
}

// libera o semáforo
int sem_up (semaphore_t *s)
{
	return 0;
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s)
{
	return 0;
}