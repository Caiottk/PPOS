// PingPongOS - PingPong Operating System - Professor: Marco Aurélio Wehrmeister

#include "ppos_disk.h"

// estrutura que define um tratador de sinal (deve ser global ou static)
static struct sigaction actDisk;
// flag que sinaliza se um disco está ocupado para operações de E/S
int tReqs;



void diskDriverBody();

int diskSched();

void disk_handle(int signum)
{
    
    disk.pacotes++;
    sem_up(&disk.cheio);
    disk.tExec += systemTime - disk.tIn;
}

int create_disk_request(int block, void *buffer, int cmd)
{
    disk_request_t *req = (disk_request_t *)malloc(sizeof(disk_request_t));

    req->prev = NULL;
    req->next = NULL;
    req->block = block;
    req->buffer = buffer;
    req->op = cmd;

    mutex_lock(&disk.mqueue);
    queue_append((queue_t **)&disk_queue, (queue_t *)req);
    mutex_unlock(&disk.mqueue);

    return 0;
}

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: numBlocksanho do disco, em blocos
// blockSize: numBlocksanho de cada bloco do disco, em bytes
int disk_mgr_init(int *numBlocks, int *blockSize)
{

    if (disk_cmd(DISK_CMD_INIT, 0, 0))
    {
        fprintf(stderr, "[PPOS error]: Falha ao iniciar o disco\n");
        return -1;
    }

    disk.numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    if (disk.numBlocks < 0)
    {
        fprintf(stderr, "[PPOS error]: Falha ao consultar o numBlocksanho do disco\n");
        return -1;
    }

    disk.blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    if (disk.blockSize < 0)
    {
        fprintf(stderr, "[PPOS error]: Falha ao consultar o numBlocksanho do block\n");
        return -1;
    }

    *numBlocks = disk.numBlocks;
    *blockSize = disk.blockSize;

    disk.init = 1;

    mutex_create(&disk.mreq);
    mutex_create(&disk.mqueue);
    sem_create(&disk.cheio, 0);
    sem_create(&disk.vazio, 0);

    disk.qtdBlocosPer = 0;
    disk.tExec = 0;

    disk.pacotes = 0;

    disk_queue = NULL;

    // registra a ação para o sinal de timer SIGUSR1
    actDisk.sa_handler = disk_handle;
    sigemptyset(&actDisk.sa_mask);
    actDisk.sa_flags = 0;
    if (sigaction(SIGUSR1, &actDisk, 0) < 0)
    {
        printf("Erro em sigaction");
        return -1;
    }

    // flag de escalonador
    disk.sched = 0;
    tReqs = disk.numBlocks*2;

    task_create(&disk.task, diskDriverBody, NULL);
    task_setprio(&disk.task, 0);

    return 0;
}

void diskDriverBody()
{
    while (disk.init == 1)
    {
        tReqs--;
        printf("%d\n", tReqs);
        if(tReqs == 0) disk.init = 0;
        sem_down(&disk.vazio);
        diskSched();
        
        
    }
    printf("Numero de blocos percorridos %d\nTempo de execução do disco %d ms\n",
           disk.qtdBlocosPer, disk.tExec);
    task_exit(0);
}

// leitura de um bloco, do disco para o buffer
int disk_block_read(int block, void *buffer)
{
    int cmd = DISK_CMD_READ;

    create_disk_request(block, buffer, cmd);

    mutex_lock(&disk.mreq);

    if (disk.pacotes == 0)
    {
        sem_up(&disk.vazio);
        sem_down(&disk.cheio);
    }
    disk.pacotes--;
    
    mutex_unlock(&disk.mreq);
    
    return 0;
}

// escrita de um bloco, do buffer para o disco
int disk_block_write(int block, void *buffer)
{
    int op = DISK_CMD_WRITE;

    // coloca tarefa na fila para acessar o disco
    create_disk_request(block, buffer, op);

    mutex_lock(&disk.mreq);

    if (disk.pacotes == 0)
    {
        sem_up(&disk.vazio);
        sem_down(&disk.cheio);
    }
    disk.pacotes = 0;
    
    mutex_unlock(&disk.mreq);

    return 0;
}

// ================== Escalonadores

disk_request_t *FCFS_sched()
{
    return disk_queue;
}

disk_request_t *SSTF_sched()
{
    int size = queue_size((queue_t *)disk_queue);
    int i = 0;
    disk_request_t *iter = disk_queue;

    int head = disk.block;
    int menor = INT_MAX;
    disk_request_t *menor_req = NULL;

    int aux;
    for (i = 0; i < size && iter != NULL; i++, iter = iter->next)
    {
        aux = abs(iter->block - head);
        if (aux < menor)
        {
            menor = aux;
            menor_req = iter;
        }
    }

    return menor_req;
}

disk_request_t *CSCAN_sched()
{
    int size = queue_size((queue_t *)disk_queue);
    int i = 0;
    disk_request_t *iter = disk_queue;

    int head = disk.block;
    int menor_frente = INT_MAX;
    int menor = INT_MAX;
    disk_request_t *menor_req = NULL;
    disk_request_t *menor_req2 = NULL;

    int aux;
    int frente = 0;
    for (i = 0; i < size && iter != NULL; i++, iter = iter->next)
    {
        aux = iter->block - head;
        if (aux >= 0)
        {
            frente = 1;
            if (aux < menor_frente)
            {
                menor_frente = aux;
                menor_req = iter;
            }
        }
        else
        {
            aux = iter->block;
            if (aux < menor)
            {
                menor = aux;
                menor_req2 = iter;
            }
        }
    }

    if (frente)
    {
        return menor_req;
    }
    else
    {
        return menor_req2;
    }
}

int diskSched()
{
    if (disk_queue == NULL){
        return 0;
    }
    disk_request_t *req, *aux;

    switch (disk.sched)
    {
    case 0:
        req = FCFS_sched();
        break;

    case 1:
        req = SSTF_sched();
        break;

    case 2:
        req = CSCAN_sched();
        break;

    default:
        req = FCFS_sched();
    }

    int req_block = req->block;
    void *req_buffer = req->buffer;
    int req_operation = req->op;

    aux = (disk_request_t *)queue_remove((queue_t **)&disk_queue, (queue_t *)req);
    free(req);

    disk.state = disk_cmd(DISK_CMD_STATUS, 0, 0);
    if (disk.state != DISK_STATUS_IDLE)
    {
        printf("disk_status = %d\n", disk.state);
        return -1;
    }
    disk.tIn = systemTime;
    disk.qtdBlocosPer += abs(req_block - disk.block);

    disk.block = req_block;
    disk.buffer = req_buffer;
    
    int result = disk_cmd(req_operation, disk.block, disk.buffer);
    if (result != 0)
    {
        printf("Falha ao ler/escrever o bloco %d %d %p %d %d\n", disk.block, result, disk.buffer, disk.numBlocks, disk.state);
        sem_up(&disk.cheio);
        return -1;
    }
}