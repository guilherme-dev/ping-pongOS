// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas


#define STACKSIZE 32768		/* tamanho de pilha das threads */

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   void *stack ;			// aponta para a pilha da tarefa
   int static_prio;
   int dinamic_prio;
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// variaveis para controle de contexto
task_t Main_task;		//tarefa main
task_t *current_task;	//ponteiro para a tarefa atual
int task_counter;		//contador para geracao de Id's de tarefas
// variaveis para uso do dispatcher e scheduler
int user_tasks;			//contador que guarda a quantidade de tarefas na fila de prontas
task_t Dispatcher;		//tarefa para o dispatcher
task_t *ready_queue;	//fila de tarefas ready


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

