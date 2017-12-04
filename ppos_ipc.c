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
    current_task->user_task = 0;
	if (!s) {
        printf("sem_up: semaforo nao existe\n");
        return -1;
    }
    s->active = 1;
	s->queue = NULL;
	s->counter = value;

    current_task->user_task = 1;
    return 0;

}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
    current_task->user_task = 0;
    if (!s || s->active == 0) {
        #ifdef DEBUG
            printf("sem_down: semaforo nao existe\n");
        #endif
        return -1;
    }
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
    current_task->user_task = 0;
    task_t *aux;
    if (!s || s->active == 0) {
        #ifdef DEBUG
            printf("sem_up: semaforo nao existe\n");
        #endif
        return -1;
    }
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
    current_task->user_task = 0;
    task_t * aux, *aux_next;
    if (!s) {
        printf("sem_destroy: semaforo nao existe\n");
        return -1;
    }
    if (s->queue != NULL) {
        aux = s->queue;
        do {
            aux_next = aux->next;
            queue_append((queue_t **) &ready_queue, queue_remove((queue_t **) &s->queue, (queue_t *) aux));
            aux = aux_next;
        } while (aux != s->queue && s->queue != NULL);
    }
    s->active = 0;
    #ifdef DEBUG
        printf("sem_destroy: destruiu semaforo\n");
    #endif
    current_task->user_task = 1;
	return 0;
}

// filas de mensagens

// cria uma fila para até max mensagens de size bytes cada
int mqueue_create (mqueue_t *queue, int max, int size)
{
    if (!queue || max <= 0 || size <= 0) {
        printf("mqueue_create: algum dos parametros foi invalido\n");
        return -1;
    }
    if (!(queue->buffer = malloc (max * size))) {
        printf("mqueue_create: erro ao alocar buffer\n");
        return -1;
    }
    queue->buffer_end = queue->buffer + max * size;
    queue->head = queue->buffer;
    queue->tail = queue->buffer;
    queue->max_msgs = max;
    queue->msg_size = size;
    queue->msg_counter = 0;
    queue->active = 1;
    sem_create (&queue->s_buffer, 1);
    sem_create (&queue->s_item, 0);
    sem_create (&queue->s_vaga, max);
    return 0;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg)
{
    if (!queue || !msg) {
        printf("mqueue_send: erro nos parametros\n");
        return -1;
    }
    if (queue->active == 0) {
        return -1;
    }
    sem_down(&queue->s_vaga);
    sem_down(&queue->s_buffer);

    memcpy(queue->head, msg, queue->msg_size);
    queue->head = (char*) queue->head + queue->msg_size;
    if (queue->head == queue->buffer_end)
        queue->head = queue->buffer;
    queue->msg_counter++;

    sem_up(&queue->s_buffer);
    sem_up(&queue->s_item);
    return 0;
}

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg)
{
    if (!queue || !msg) {
        printf("mqueue_recv: erro nos parametros\n");
        return -1;
    }
    if (queue->active == 0) {
        return -1;
    }
    sem_down (&queue->s_item);
    sem_down (&queue->s_buffer);

    memcpy(msg, queue->tail, queue->msg_size);
    queue->tail = (char*) queue->tail + queue->msg_size;
    if(queue->tail == queue->buffer_end)
        queue->tail = queue->buffer;
    queue->msg_counter--;

    sem_up (&queue->s_buffer);
    sem_up (&queue->s_vaga);
    return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue)
{
    if (!queue) {
        printf("mqueue_destroy: erro nos parametros\n");
        return -1;
    }
    queue->active = 0;
    sem_destroy(&queue->s_buffer);
    sem_destroy(&queue->s_item);
    sem_destroy(&queue->s_vaga);
    #ifdef DEBUG
        printf("mqueue_destroy: destruiu semaforos\n");
    #endif
    return 0;
}

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue)
{
    if (!queue)
        return -1;
    #ifdef DEBUG
        printf("mqueue_msgs: num de msgs na fila: %d\n", queue->msg_counter);
    #endif
    return queue->msg_counter;
}
