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
    disk.pacotes++; // indica que a requisição foi concluída 
    sem_up(&disk.cheio); // sinaliza que uma requisição foi concluída
    disk.tExec += systemTime - disk.tIn; // calcula o tempo de operação
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
// numBlocks: nº blocos do disco, em blocos
// blockSize: tamanho de um bloco do disco, em bytes
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
    tReqs = disk.numBlocks * 2;

    task_create(&disk.task, diskDriverBody, NULL);
    task_setprio(&disk.task, 0);

    return 0;
}
// corpo principal
void diskDriverBody()
{
    while (disk.init == 1)
    {
        // tReqs--; // 
        // if (tReqs == 0)
        //     disk.init = 0;
        sem_down(&disk.vazio); // sinaliza que há uma requisição a ser processada
        diskSched(); // próxima requisição e como ela vai ser atendida de acordo com o escalonador
    }
    printf("Numero de blocos percorridos %d\nTempo de execução do disco %d ms\n",
           disk.qtdBlocosPer, disk.tExec);
    task_exit(0); // encerra a tarefa do driver de disco
}

// leitura de um bloco, do disco para o buffer
int disk_block_read(int block, void *buffer)
{
    if (block < 0 || !buffer)
        return -1;

    create_disk_request(block, buffer, DISK_CMD_READ);

    mutex_lock(&disk.mreq); // garante que apenas uma requisição seja processada por vez

    if (disk.pacotes == 0) // se a fila de requisições está vazia
    {
        sem_up(&disk.vazio); // sinaliza que há uma requisição disponível para ser processada
        sem_down(&disk.cheio); // aguarda a conclusão da requisição
    }
    disk.pacotes--;

    mutex_unlock(&disk.mreq);

    return 0;
}

// escrita de um bloco, do buffer para o disco
int disk_block_write(int block, void *buffer)
{
    if (block < 0 || !buffer)
        return -1;

    create_disk_request(block, buffer, DISK_CMD_WRITE);

    mutex_lock(&disk.mreq);

    if (disk.pacotes == 0)
    {
        sem_up(&disk.vazio);
        sem_down(&disk.cheio);
    }
    disk.pacotes --;

    mutex_unlock(&disk.mreq);

    return 0;
}

// ================== Escalonadores

disk_request_t *FCFS_sched()
{
    return disk_queue;
}

disk_request_t *SSTF_sched(int size, disk_request_t *it)
{
    int head = disk.block;
    int menor = INT_MAX;
    disk_request_t *menor_req = NULL;

    int aux;
    for (int i = 0; i < size && it != NULL; i++, it = it->next)
    {
        aux = abs(it->block - head);
        if (aux < menor)
        {
            menor = aux;
            menor_req = it;
        }
    }

    return menor_req;
}

disk_request_t *CSCAN_sched(int size, disk_request_t *it)
{
    int head = disk.block;
    int menor_frente = INT_MAX;
    int menor = INT_MAX;
    disk_request_t *menor_req = NULL;
    disk_request_t *menor_req2 = NULL;

    int aux;
    int frente = 0;
    for (int i = 0; i < size && it != NULL; i++, it = it->next)
    {
        aux = it->block - head;
        if (aux >= 0)
        {
            frente = 1;
            if (aux < menor_frente)
            {
                menor_frente = aux;
                menor_req = it;
            }
        }
        else
        {
            aux = it->block;
            if (aux < menor)
            {
                menor = aux;
                menor_req2 = it;
            }
        }
    }

    if (frente)
        return menor_req;

    else
        return menor_req2;
}

int diskSched()
{
    if (disk_queue == NULL) 
    {
        return 0; // indica que não ouve necessidade de processar nenhuma requisição
    }
    disk_request_t *req, *aux; // armazenam a requisição selecionada e requisição temporária removida da fila

    int size = queue_size((queue_t *)disk_queue);
    disk_request_t *iter = disk_queue;
    switch (disk.sched)
    {
    case 0:
        req = FCFS_sched();
        break;

    case 1:
        req = SSTF_sched(size, iter);
        break;

    case 2:
        req = CSCAN_sched(size, iter);
        break;

    default:
        req = FCFS_sched();
    }

    int req_block = req->block;
    void *req_buffer = req->buffer;
    int req_operation = req->op;

    aux = (disk_request_t *)queue_remove((queue_t **)&disk_queue, (queue_t *)req); // remove a requisição da fila 
    free(req);

    disk.state = disk_cmd(DISK_CMD_STATUS, 0, 0);
    if (disk.state != DISK_STATUS_IDLE)
    {
        printf("disk_status = %d\n", disk.state);
        return -1;
    }
    disk.tIn = systemTime; // get do início do tempo da requisição
    disk.qtdBlocosPer += abs(req_block - disk.block); // calcula o tanto de blocos que foram percorridos para cada requisição

    disk.block = req_block; // atualiza o bloco atual de acordo com a requisição
    disk.buffer = req_buffer; // atualiza o buffer com o buffer da requisição

    if (disk_cmd(req_operation, disk.block, disk.buffer) < 0) // executa a operação solicitada pela requisição no bloco selecionado com o buffer correspondente
    {
        printf("Falha ao processar requisição(E/S) no bloco %d %p %d %d\n", disk.block, disk.buffer, disk.numBlocks, disk.state);
        sem_up(&disk.cheio); // libera o semáforo se a fila de requisições está cheia
        return -1;
    }
}