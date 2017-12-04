// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// interface do gerente de disco rígido (block device driver)
#include <stdio.h>
#include <stdlib.h>
#include "ppos_disk.h"
#include "ppos.h"


// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize)
{
    if (numBlocks <= 0 || blockSize <= 0) {
        printf("disk_mgr_init: parametros invalidos\n");
        return -1;
    }
    // inicializa um disco (operacao sincrona)
    if (disk_cmd (DISK_CMD_INIT, 0, 0) < 0) {
        printf("disk_mgr_init: erro ao inicializar disco\n");
        return -1;
    }

    *blockSize = disk_cmd (DISK_CMD_BLOCKSIZE, 0, 0);
    *numBlocks = disk_cmd (DISK_CMD_DISKSIZE, 0, 0);

    if (*blockSize < 0 || *numBlocks < 0) {
        printf("disk_mgr_init: tamanho do disco ou blocos invalidos\n");
        return -1;
    }

    task_create(&Disk_task, diskDriverBody, " ");
    #ifdef DEBUG
    	printf("disk_mgr_init: criou tarefa disk manager\n");
    #endif

    printf("disk delay %d\n", disk_cmd (DISK_CMD_DELAYMAX, 0, 0));
    //nao tem dependencia, assim nao eh removido da fila de suspensas pela task_exit()
    Disk_task.dependency = -1;
    // Disk_mgr.disk_suspended = 1;
    // queue_append((queue_t **) &suspended_queue, queue_remove((queue_t **)&ready_queue, (queue_t *) &Disk_task));

    sem_create(&Disk_mgr.s_disk, 1);

    // registra ação para o sinal de timer SIGUSR1
    disk_action.sa_handler = sigdisk_handler;
    sigemptyset (&disk_action.sa_mask);
    disk_action.sa_flags = 0;
    if (sigaction (SIGUSR1, &disk_action, 0) < 0)
    {
        printf("disk_mgr_init: Erro em sigaction: SIGUSR1\n");
        exit (-1);
    }
    #ifdef DEBUG
    	printf("disk_mgr_init: criou signal handler para o disco\n");
    #endif

    return 0;

}

//cria uma requisicao de disco
disk_requests_t *disk_request_create(task_t *task, int req_type, int block, void *buffer)
{

    disk_requests_t *disk_request;

    if (! (disk_request = (disk_requests_t *) malloc(sizeof(disk_requests_t)))) {
        printf("disk_request_create: erro ao alocar memoria para requisicao\n");
        exit(-1);
    }

    disk_request->task = task;
    disk_request->req_type = req_type;
    disk_request->block = block;
    disk_request->buffer = buffer;

    return disk_request;
}

void disk_request_destroy (disk_requests_t * disk_request)
{
    free(disk_request);
}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer)
{
    disk_requests_t * d_req;

    // obtém o semáforo de acesso ao disco
    sem_down(&Disk_mgr.s_disk);
    // inclui o pedido na fila_disco
    d_req = disk_request_create(current_task, DISK_CMD_READ, block, buffer);
    queue_append((queue_t **) &Disk_mgr.disk_queue, (queue_t *) d_req);
    if (Disk_mgr.disk_suspended)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        queue_append((queue_t **)&ready_queue, queue_remove((queue_t **)&suspended_queue, (queue_t *) &Disk_task));
        Disk_mgr.disk_suspended = 0;
        #ifdef DEBUG
            printf("disk_block_read: colocou gerente de disco na fila de prontas\n");
        #endif
    }

    // libera semáforo de acesso ao disco
    sem_up(&Disk_mgr.s_disk);

    // suspende a tarefa corrente (retorna ao dispatcher)
    current_task->dependency = -1;
    queue_append((queue_t **) &suspended_queue, (queue_t*) current_task);
    task_switch(&Dispatcher);

    return 0;
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer)
{
    disk_requests_t * d_req;
    task_t *aux;

    // obtém o semáforo de acesso ao disco
    sem_down(&Disk_mgr.s_disk);
    // inclui o pedido na fila_disco
    d_req = disk_request_create(current_task, DISK_CMD_WRITE, block, buffer);
    queue_append((queue_t **)&Disk_mgr.disk_queue, (queue_t *) d_req);

    if (Disk_mgr.disk_suspended)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        aux = &Disk_task;
        queue_append((queue_t **)&ready_queue, queue_remove((queue_t **)&suspended_queue, (queue_t *) aux));
        Disk_mgr.disk_suspended = 0;
        #ifdef DEBUG
            printf("disk_block_read: colocou gerente de disco na fila de prontas\n");
        #endif
    }

    // libera semáforo de acesso ao disco
    sem_up(&Disk_mgr.s_disk);

    // suspende a tarefa corrente (retorna ao dispatcher)
    current_task->dependency = -1;
    queue_append((queue_t **) &suspended_queue, (queue_t*) current_task);
    task_switch(&Dispatcher);

    return 0;
}

//tarefa gerente de disco
void diskDriverBody ()
{
    disk_requests_t * d_req;
    task_t *aux;
   while (1)
   {
        // obtém o semáforo de acesso ao disco
        sem_down(&Disk_mgr.s_disk);
        // se foi acordado devido a um sinal do disco
        if (Disk_mgr.sig_recv == 1)
        {
            // acorda a tarefa cujo pedido foi atendido
            printf("Disk_mgr.sig_recv> %d\n", Disk_mgr.sig_recv);
            aux = suspended_queue;
            queue_append((queue_t **)&ready_queue, queue_remove((queue_t **)&suspended_queue, (queue_t *) aux));
            queue_remove((queue_t **)&Disk_mgr.disk_queue, (queue_t *) d_req);
            free(d_req);
            Disk_mgr.sig_recv = 0;

        }
        // se o disco estiver livre e houver pedidos de E/S na fila
        if ((disk_cmd (DISK_CMD_STATUS, 0, 0) == 1) && Disk_mgr.disk_queue != NULL)
        {
            // escolhe na fila o pedido a ser atendido, usando FCFS
            // solicita ao disco a operação de E/S, usando disk_cmd()
            d_req = Disk_mgr.disk_queue;

            if (disk_cmd (d_req->req_type, d_req->block, d_req->buffer) < 0 ) {
                printf("diskDriverBody: erro ao agendar pedido ao disco\n");
            }
        }
        // libera o semáforo de acesso ao disco
        sem_up(&Disk_mgr.s_disk);

        // suspende a tarefa corrente (retorna ao dispatcher)
        queue_append((queue_t **) &suspended_queue, (queue_t*) current_task);
        task_switch(&Dispatcher);
   }
}

//tratador de sinais do disco
void sigdisk_handler (int signum)
{
    Disk_mgr.sig_recv = 1;
    printf("Received signal\n");
    queue_append((queue_t **)&ready_queue, queue_remove((queue_t **)&suspended_queue, (queue_t *) &Disk_task));
    Disk_mgr.disk_suspended = 0;
}
