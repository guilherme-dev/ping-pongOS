// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// Núcleo para as aplicações
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

//Prototipos de funcoes nao globais
void dispatcher_body ();
task_t * scheduler ();
void sigalrm_handler (int signum);

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
	// desativa o buffer da saida padrao (stdout), usado pela função printf
	setvbuf (stdout, 0, _IONBF, 0);
    current_timer = 0;

	//inicializa task Main
	Main_task.prev = Main_task.next = NULL;
	Main_task.id = 0;
    Main_task.quantum = QUANTUMSIZE;
    Main_task.user_task = 1;
    if (!Main_task.static_prio) {
        Main_task.static_prio = Main_task.dinamic_prio = 0;
    }
    Main_task.exec_time = Main_task.cpu_time_sum = Main_task.cpu_time = systime();
    Main_task.activations = 0;


	current_task = &Main_task;
	task_counter = 0;

	//inicializa dispatcher
	task_create(&Dispatcher, dispatcher_body, " ");
    #ifdef DEBUG
    	printf("ppos_init: inicializou estruturas\n");
    #endif

    // registra ação para o sinal de timer SIGALRM
    action.sa_handler = sigalrm_handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        printf("Erro em sigaction: SIGALRM\n");
        exit (-1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 10 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
      printf ("Erro em setitimer: ") ;
      exit (1) ;
    }
    #ifdef DEBUG
        printf("ppos_init: criou temporizador\n");
    #endif

    //Utilizando task_yield para colocar a Main na fila de prontas e passar o controle ao Dispatcher
    //Nesse momento a fila conterá apenas a Main, que voltará para execução logo após ppo_init()
    task_yield();
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

	getcontext (&task->context);

	task->stack = malloc (STACKSIZE);
	if (task->stack)
	{
		task->context.uc_stack.ss_sp = task->stack;
    	task->context.uc_stack.ss_size = STACKSIZE;
    	task->context.uc_stack.ss_flags = 0;
    	task->context.uc_link = 0;
	}
	else
	{
		printf ("Erro na criação da pilha: \n");
		exit (1);
	}

	makecontext (&task->context, (void*)(*start_func), 1, arg);


	//controle para geracao de Id's
	task_counter++;
	task->id = task_counter;

    //atributos da tarefa
    if (!task->static_prio) {
        task->static_prio = task->dinamic_prio = 0;
    }

    if (task->id > 1) {
        task->quantum = QUANTUMSIZE;
        task->user_task = 1;
    } else {
        task->user_task = 0; //se dispatcher
    }

    task->exec_time = task->cpu_time_sum = task->cpu_time = systime();
    task->activations = 0;

	#ifdef DEBUG
		printf("task_create: criou tarefa %d\n", task->id);
	#endif
	if (task->user_task)
		queue_append((queue_t **) &ready_queue, (queue_t *) task);
    	#ifdef DEBUG
    		printf("task_create: inseriu tarefa %d na fila de prontas\n", task->id);
    	#endif


	return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
    unsigned int time_now = systime();
	#ifdef DEBUG
		printf("task_exit: codigo %d para encerrar tarefa %d\n", exitCode, current_task->id);
	#endif

    current_task->cpu_time_sum = current_task->cpu_time_sum + (time_now - current_task->cpu_time);
    current_task->exec_time = time_now - current_task->exec_time;
    printf("Task %4d exit: running time %4d ms, cpu time  %4d ms, %d activations\n",
            current_task->id,
            current_task->exec_time,
            current_task->cpu_time_sum,
            current_task->activations );
    //Se a tarefa atual eh o dispathcer, volta pra Main
    //A Main precisa ter um exit(0)
	if (current_task->id == 1)
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
    unsigned int time_now = systime();
	if (!task)
	{
		printf("task_switch: task_t *task nao definida\n");
		return -1;
	}
    //Contabilizacao
    current_task->cpu_time_sum += time_now - current_task->cpu_time;
    task->cpu_time = time_now;
    task->activations++;

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

	if (current_task->id != 1)		//Nao insere Dispatcher
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
	else {
		task->static_prio = task->dinamic_prio = prio;
    }
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
    #ifdef DEBUG
        printf("Tamanho da fila de prontas: %d\n", user_tasks);
    #endif
	while (queue_size((queue_t *) ready_queue) > 0)
	{
		next = scheduler();
		if (next)
		{
			queue_remove((queue_t **) &ready_queue, (queue_t *) next);
            next->quantum = QUANTUMSIZE;
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
		if (aux->dinamic_prio < high_prio->dinamic_prio) {
			high_prio = aux;
        } else if (aux->dinamic_prio == high_prio->dinamic_prio) {
            if (aux->static_prio < high_prio->static_prio )
                high_prio = aux;
        }
		#ifdef DEBUG2
			printf("scheduler: task_id %d com prioridade %d\n", aux->id, aux->dinamic_prio);
		#endif
        aux->dinamic_prio--;
	} while (aux->next != first);

	#ifdef DEBUG
		printf("scheduler: prioridade mais alta:%d\n", high_prio->dinamic_prio);
	#endif
	//reseta prioridade dinamica
	high_prio->dinamic_prio = high_prio->static_prio;
	return high_prio;
}

//trador do sinal do timer
void sigalrm_handler (int signum)
{
    // #ifdef DEBUG
    //     printf("Signal!\n");
    // #endif
    current_timer++;
    if (current_task->user_task) {
        if (current_task->quantum > 0) {
            current_task->quantum--;
        }
        else {
            task_yield();
        }
    }
}

unsigned int systime ()
{
    return current_timer;
}
