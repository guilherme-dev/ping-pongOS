// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)
#include "ppos.h"
#include "hard_disk.h"
#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef struct disk_requests_t
{
    struct disk_requests_t *prev, *next;
    task_t *task;
    int req_type;
    int block;
    void *buffer;
} disk_requests_t;

// estrutura que representa um disco no sistema operacional
typedef struct
{
  semaphore_t s_disk;
  disk_requests_t *disk_queue;	//fila do disco
  int sig_recv;
  int disk_suspended;
} disk_t ;

task_t Disk_task;        // tarefa gerente de disco
disk_t Disk_mgr;

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction disk_action;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;


disk_requests_t * disk_request_create(task_t *task, int req_type, int block, void *buffer);


void disk_request_destroy (disk_requests_t * disk_request);

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

//tarefa gerente de disco
void diskDriverBody ();

//tratador de sinais do disco
void sigdisk_handler (int signum);

#endif
