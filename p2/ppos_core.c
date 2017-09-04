// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// Núcleo para as aplicações
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0) ;
	Main_task.prev = Main_task.next = NULL ;
	Main_task.id = 0 ;
	current_task = &Main_task ;
	task_counter = 0;
	#ifdef DEBUG
		printf("ppos_init: inicializou estruturas\n");
	#endif
}

// gerência de tarefas =========================================================
// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task, void (*start_func)(void *), void *arg)
{
	if (!task)
	{
		printf("task_create: task_t *task nao definida\n");
		return -1;
	}

	getcontext (&task->context) ;

	task->stack = malloc (STACKSIZE) ;
	if (task->stack) 
	{
		task->context.uc_stack.ss_sp = task->stack ;
    	task->context.uc_stack.ss_size = STACKSIZE ;
    	task->context.uc_stack.ss_flags = 0 ;
    	task->context.uc_link = 0 ;
	}
	else 
	{
		printf ("Erro na criação da pilha: \n") ;
		exit (1) ;
	}

	makecontext (&task->context, (void*)(*start_func), 1, arg) ;
	task_counter++;
	task->id = task_counter;
	#ifdef DEBUG
		printf("task_create: gerou contexto para task %d\n", task->id);
	#endif
	return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
	#ifdef DEBUG
		printf("task_exit: codigo %d para encerrar tarefa %d\n", exitCode, current_task->id);
	#endif
	task_switch(&Main_task);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) 
{
	if (!task)
	{
		printf("task_switch: task_t *task nao definida\n");
		return -1;
	}
	task_t *aux = current_task;
	current_task = task;
	#ifdef DEBUG
		printf("task_switch: trocando contexto %d -> %d\n", aux->id, task->id);
	#endif
	swapcontext(&aux->context, &task->context);
	return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
	#ifdef DEBUG
		printf("task_id: tarefa atual: %d\n", current_task->id);
	#endif
	return current_task->id;
}
