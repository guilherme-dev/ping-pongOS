// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// Núcleo para as aplicações
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

//Prototipos
void dispatcher_body ();
task_t * scheduler ();

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
	// desativa o buffer da saida padrao (stdout), usado pela função printf 
	setvbuf (stdout, 0, _IONBF, 0) ;
	//inicializa task Main
	Main_task.prev = Main_task.next = NULL ;
	Main_task.id = 0 ;
	current_task = &Main_task ;
	task_counter = 0;

	//inicializa dispatcher
	task_create(&Dispatcher, dispatcher_body, " ");
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
	//controle para geracao de Id's	
	task_counter++;	
	task->id = task_counter;
	#ifdef DEBUG
		printf("task_create: criou tarefa %d\n", task->id);
	#endif
	if (task->id > 1)
		queue_append((queue_t **) &ready_queue, (queue_t *) task);
	#ifdef DEBUG
		printf("task_create: inseriu tarefa %d na fila de prontas\n", task->id);
	#endif
	return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
	#ifdef DEBUG
		printf("task_exit: codigo %d para encerrar tarefa %d\n", exitCode, current_task->id);
	#endif
	//Se a tarefa atual eh o dispathcer, volta pra Main
	if (current_task->id <= 1) 
	{
		task_switch(&Main_task);
	} 
	else
	{
		user_tasks--;
		task_switch(&Dispatcher);
	}
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

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield ()
{

	//Insere tarefa atual na fila de prontas
	#ifdef DEBUG
		printf("task_yield: current_task> %d\n", current_task->id);
	#endif
	if (current_task->id > 1)		//Nao insere a Main na fila de prontas
		queue_append((queue_t **) &ready_queue, (queue_t*) current_task);
	task_switch(&Dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio)
{
	if (prio < -20 || prio > 20)
	{
		#ifdef DEBUG
			printf("task_setprio: valor de parametro prio invalido\n");
		#endif
		return;
	}
	if (!task)
		current_task->static_prio = current_task->dinamic_prio = prio;
	else
		task->static_prio = task->dinamic_prio = prio;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task)
{
	if (!task)
		return current_task->static_prio;
	else
		return task->static_prio;
}


//Body da tarefa dispatcher
void dispatcher_body ()
{
	task_t *next;
	//define tamanho da fila de prontas
	user_tasks = queue_size((queue_t *) ready_queue);
	while (user_tasks > 0)
	{
		next = scheduler();
		if (next)
		{
			queue_remove((queue_t **) &ready_queue, (queue_t *) next);
			task_switch (next); // transfere controle para a tarefa "next"
		}
	}
	   task_exit(0) ; // encerra a tarefa dispatcher
}

//Scheduler por prioridades
task_t * scheduler ()
{
	task_t *high_prio, *aux, *first;		//variaveis de controle
	aux = first = high_prio = ready_queue;
	do
	{
		aux = aux->next;
		//Se prioridade dinamico menor, troca e diminui o alfa
		if (aux->dinamic_prio <= high_prio->dinamic_prio) {
			high_prio->dinamic_prio--;
			high_prio = aux;
		//se nao eh menor, diminui o alfa
		} else if (aux->dinamic_prio > -21) { //-21 para caso existam duas tarefas -20
			aux->dinamic_prio--;
		}
		#ifdef DEBUG
			printf("scheduler: task_id %d com prioridade %d\n", aux->id, aux->dinamic_prio);
		#endif
	} while (aux->next != first);

	#ifdef DEBUG
		printf("scheduler: prioridade mais alta:%d\n", high_prio->dinamic_prio);
	#endif
	//reseta prioridade dinamica 
	high_prio->dinamic_prio = high_prio->static_prio; 
	return high_prio;
}