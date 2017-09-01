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
}


// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task, void (*start_func)(void *), void *arg)
{
	if (!task)
		return -1;

	getcontext (&task->context) ;

	task->stack = malloc (STACKSIZE) ;
	if (task->stack) {
		task->context.uc_stack.ss_sp = task->stack ;
    	task->context.uc_stack.ss_size = STACKSIZE ;
    	task->context.uc_stack.ss_flags = 0 ;
    	task->context.uc_link = 0 ;
		// printf("%p\n", task->context.uc_link);
	}
	else {
		printf ("Erro na criação da pilha: \n") ;
		exit (1) ;
	}

	makecontext (&task->context, (void*)(*start_func), 1, arg) ;

	return 0;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{

}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) 
{
	return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
	return 0;
}
