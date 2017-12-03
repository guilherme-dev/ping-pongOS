// PingPongOS - PingPong Operating System
// Guilherme Gomes dos Santos - GRR20124499
// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__


// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize)
{
    if (!numBlocks <= 0 || !blockSize <= 0) {
        printf("disk_mgr_init: parametros invalidos\n", );
        return -1;
    }
    // inicializa um disco (operacao sincrona)
    if (disk_cmd (DISK_CMD_INIT, 0, 0) < 0) {
        printf("disk_mgr_init: erro ao inicializar disco\n", );
        return -1;
    }

    *blockSize = disk_cmd (DISK_CMD_BLOCKSIZE, 0, 0);
    *numBlocks = disk_cmd (DISK_CMD_DISKSIZE, 0, 0);

    task_create(&Disk_mgr, diskDriverBody, " ");
    #ifdef DEBUG
    	printf("disk_mgr_init: criou tarefa disk manager\n");
    #endif

    if (*blockSize < 0 || *numBlocks < 0) {
        printf("disk_mgr_init: tamanho do disco ou blocos invalidos\n", );
        return -1;
    }
    return 0;

}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer)
{
    // obtém o semáforo de acesso ao disco

    // inclui o pedido na fila_disco

    if (gerente de disco está dormindo)
    {
       // acorda o gerente de disco (põe ele na fila de prontas)
    }

    // libera semáforo de acesso ao disco

    // suspende a tarefa corrente (retorna ao dispatcher)
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer)
{

}

#endif
