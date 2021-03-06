// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas


#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define QUANTUMSIZE 30

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   void *stack ;			// aponta para a pilha da tarefa
   int static_prio;         //prioridade estatica
   int dinamic_prio;        //prioridade dinamica
   int quantum;             //quantum
   int user_task;           // 1 - tarefa de usuario, 0 - tarefa de sistema
   int status;              // 0 - finalizada, 1 - ativa (pronta, execução ou suspensa)
   int exit_code;
   int dependency;          // Id da tarefa da qual esta depende para sair do modo suspenso
   int awake;                // tempo a ser acordada
   unsigned int exec_time;
   unsigned int cpu_time_sum;
   unsigned int cpu_time;
   unsigned int activations;

} task_t ;

// variaveis para controle de contexto
task_t Main_task;		//tarefa main
task_t *current_task;	//ponteiro para a tarefa atual
// int task_counter; 		//contador para geracao de Id's de tarefas

// variaveis para uso do dispatcher e scheduler
int user_tasks;			//contador que guarda a quantidade de tarefas na fila de prontas
task_t Dispatcher;		//tarefa para o dispatcher
task_t *ready_queue;	//fila de tarefas ready
task_t *suspended_queue; //fila de tarefas suspensas
task_t *sleep_queue;   //fila de tarefas adormecidas
unsigned int current_timer; //relogio

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização to timer
struct itimerval timer;

// estrutura que define um semáforo
typedef struct
{
  int counter;
  int active;
  task_t *queue;
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
    void *buffer ;
    void *buffer_end;
    void *head;
    void *tail;
    int max_msgs ;
    int msg_size ;
    int msg_counter ;
    int active;
    semaphore_t s_buffer ;
    semaphore_t s_item ;
    semaphore_t s_vaga ;
} mqueue_t ;

#endif
