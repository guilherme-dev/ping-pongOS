// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// Núcleo para as aplicações
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

// Prototipos de funcoes gerais ================================================
void dispatcher_body ();
task_t * scheduler ();
void sigalrm_handler (int signum);

// funções gerais ==============================================================

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
	// task_counter = 0;
    user_tasks = 1;
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
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
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
    //atributos da tarefa
    if (!task->static_prio) {
        task->static_prio = task->dinamic_prio = 0;
    }

    task->status = 1;

    if (task != &Dispatcher) {
        task->id = queue_size((queue_t *) ready_queue) + 2;
        task->quantum = QUANTUMSIZE;
        task->user_task = 1;
    } else {
        task->id = 1;
        task->user_task = 0; //se dispatcher
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



    task->exec_time = task->cpu_time_sum = task->cpu_time = systime();
    task->activations = 0;

	#ifdef DEBUG
        printf("Tamanho da fila: %d em %d\n", queue_size((queue_t *) ready_queue), systime());
		printf("task_create: criou tarefa %s\n", (char *)arg);
	#endif
	if (task->user_task) {
        user_tasks++;
		queue_append((queue_t **) &ready_queue, (queue_t *) task);
    	#ifdef DEBUG
    		printf("task_create: inseriu tarefa %d na fila de prontas\n", task->id);
    	#endif
	}



	return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
	unsigned int time_now = systime();
	task_t *aux, *first;
	#ifdef DEBUG
		printf("task_exit: codigo %d para encerrar tarefa %d\n", exitCode, current_task->id);
	#endif
	current_task->exit_code = exitCode;
	current_task->status = 0;

    current_task->cpu_time_sum = current_task->cpu_time_sum + (time_now - current_task->cpu_time);
    current_task->exec_time = time_now - current_task->exec_time;
    printf("Task %4d exit: running time %4d ms, cpu time  %4d ms, %d activations\n",
            current_task->id,
            current_task->exec_time,
            current_task->cpu_time_sum,
			current_task->activations );
	//Se existem tarefas aguardando a tarefa atual, estas tarefas devem
	//voltar para a fila de prontas
	aux = first = suspended_queue;
    if (suspended_queue) {
        do {
    		if (aux->dependency == current_task->id) {
    			queue_append((queue_t **)&ready_queue, queue_remove((queue_t **)&suspended_queue, (queue_t *) aux));
    			#ifdef DEBUG
    				printf("task_exit: tarefa %d entrou na fila de prontas\n", aux->id);
    			#endif
    		}
    		aux = aux->next;
    	} while (aux != first);
    }

    //Se a tarefa atual eh o dispathcer, volta pra Main
    //A Main precisa ter um exit(0)
	if (current_task->id == 1) {
		task_switch(&Main_task);
	} else {
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

// operações de escalonamento ==================================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield ()
{
	//Insere tarefa atual na fila de prontas
	#ifdef DEBUG
		printf("task_yield: current_task> %d\n", current_task->id);
	#endif

	if (current_task->id != 1) {		//Nao insere Dispatcher
		queue_append((queue_t **) &ready_queue, (queue_t*) current_task);
	}
	task_switch(&Dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio)
{
	if (prio < -20 || prio > 20) {
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

// Body da tarefa dispatcher
void dispatcher_body ()
{
	task_t *next, *aux, *aux_next;
	//define tamanho da fila de prontas
    #ifdef DEBUG
        printf("dispatcher: Tamanho da fila de prontas: %d\n", queue_size((queue_t *) ready_queue));
    #endif

	while (queue_size((queue_t *) ready_queue) > 0 || queue_size((queue_t *) sleep_queue) > 0 || user_tasks > 0) {
		next = scheduler();
		if (next) {
			queue_remove((queue_t **) &ready_queue, (queue_t *) next);
            next->quantum = QUANTUMSIZE;
			task_switch (next); // transfere controle para a tarefa "next"
            if (next->status == 0)
                free(next->context.uc_stack.ss_sp);

		}
        //verifica fila de tarefas adormecidas
        if (sleep_queue) {
            aux = sleep_queue;
            do {
                if (systime() >= aux->awake) {
                    aux_next = aux->next;   //Como queue_remove retira os ponteiros de aux, preciso salva-los
                    #ifdef DEBUG
                        printf("dispatcher: acordou tarefa %d\n", aux->id);
                    #endif
                    queue_append((queue_t **) &ready_queue, queue_remove((queue_t **) &sleep_queue, (queue_t *) aux));
                    aux = aux_next;
                } else {
                    aux = aux->next;
                }
            } while (aux != sleep_queue && sleep_queue != NULL);
        }
	}
	task_exit(0) ; // encerra a tarefa dispatcher
}


// operações de sincronização ==================================================

// Scheduler por prioridades
task_t * scheduler ()
{
    task_t *high_prio, *aux, *first;		//variaveis de controle
    aux = first = high_prio = ready_queue;
    if (queue_size((queue_t *) ready_queue) < 1)
        return NULL;


	do {
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
		aux = aux->next;
	} while (aux != first);

	#ifdef DEBUG2
		printf("scheduler: prioridade mais alta:%d\n", high_prio->dinamic_prio);
	#endif
	//reseta prioridade dinamica
	high_prio->dinamic_prio = high_prio->static_prio;
	return high_prio;
}

// trador do sinal do timer
void sigalrm_handler (int signum)
{
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

// a tarefa corrente aguarda o encerramento de outra task
int task_join (task_t *task)
{
	if (task->status == 0 || current_task->id == 1) {
		return -1;
	} else {
		current_task->dependency = task->id;
		queue_append((queue_t **) &suspended_queue, (queue_t*) current_task);
		task_switch(&Dispatcher);
	}
	current_task->dependency = -1;
	return task->exit_code;
}

// operações de gestão do tempo ================================================

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t)
{
    if (t < 0)
        return;
    current_task->awake = systime() + t ;
    queue_append((queue_t **) &sleep_queue, (queue_t *) current_task);
    #ifdef DEBUG
        printf("task_sleep: inseriu task %d na fila de adormecidas\n", current_task->id);
    #endif
    task_switch(&Dispatcher);
}

// retorna o relógio atual (em milisegundos)
unsigned int systime ()
{
    return current_timer;
}


// operações de IPC ============================================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value)
{
    current_task->user_task = 0;
	if (!s) {
        printf("sem_up: semaforo nao existe\n");
        return -1;
    }
	s->queue = NULL;
	s->counter = value;

    current_task->user_task = 1;
    return 0;

}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
    if (!s) {
        printf("sem_down: semaforo nao existe\n");
        return -1;
    }
    current_task->user_task = 0;

    s->counter = s->counter - 1;
    if (s->counter < 0) {
        #ifdef DEBUG
            printf("sem_down: inseriu %d na fila do semaforo\n", current_task->id);
        #endif
        queue_append((queue_t **) &s->queue, (queue_t *) current_task);
        task_switch(&Dispatcher);
    }
    current_task->user_task = 1;

	return 0;
}

// libera o semáforo
int sem_up (semaphore_t *s)
{
    task_t *aux;
    if (!s) {
        printf("sem_up: semaforo nao existe\n");
        return -1;
    }
    current_task->user_task = 0;

    s->counter = s->counter + 1;
    if (s->queue != NULL && s->counter  <= 0) {
        aux = s->queue;
        queue_append((queue_t **) &ready_queue, queue_remove((queue_t **) &s->queue, (queue_t * ) aux));
        #ifdef DEBUG
            printf("sem_up: inseriu %d na fila de prontas\n", current_task->id);
        #endif
    }
    current_task->user_task = 1;
	return 0;
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s)
{
    task_t * aux, *aux_next;
    if (!s) {
        printf("sem_destroy: semaforo nao existe\n");
        return -1;
    }
    current_task->user_task = 0;
    if (s->queue != NULL) {
        aux = s->queue;
        do {
            aux_next = aux->next;
            queue_append((queue_t **) &ready_queue, queue_remove((queue_t **) &s->queue, (queue_t *) aux));
            aux = aux_next;
        } while (aux != s->queue && s->queue != NULL);
    }
    s = NULL;

    current_task->user_task = 1;
	return 0;
}
