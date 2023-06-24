// PingPongOS - PingPong Operating System - Professor: Marco Aurélio Wehrmeister

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "ppos.h"
#include "ppos_disk.h"
#include "disk.h"
#include "ppos-core-globals.h"

int cont = 0;
// task reponsável pelo disk driver
task_t tDisk;
// o disco de fato
disk_t disk;

// estrutura que define um tratador de sinal (deve ser global ou static)
static struct sigaction actDisk;
// struct that defines the timer
struct itimerval timer;

// cria um novo request para o driver de disco
disk_request_t *create_request(int block, void *buff, int op)
{
    disk_request_t *req = (disk_request_t *)malloc(sizeof(disk_request_t));
    if (!req)
        exit(-1);
    req->prev = NULL;
    req->next = NULL;
    req->task = taskExec;
    req->block = block;
    req->buffer = buff;
    req->op = op;

    return req;
}

// tratador de sinal
static void disk_handle(int signum)
{
    printf("***\n");
    if (tDisk.state == 'S')
    {
        task_resume((task_t *)&tDisk);
    }
    disk.sig = 1;
}

// tirada da página oficial do projeto do Maziero a fim de simplicar a implementação
// A tarefa gerente de disco é responsável por tratar os pedidos de leitura/escrita das tarefas e os sinais gerados pelo disco.
// Ela é uma tarefa de sistema, similar ao dispatcher, e tem +/- o seguinte comportamento:
static void diskDriverBody()
{
    disk_request_t *req;
    while (1)
    {
        // obtém o semáforo de acesso ao disco
        sem_down(&(disk.ac));
        // se foi acordado devido a um sinal do disco
        if (disk.sig == 1)
        {
            // acorda a tarefa cujo pedido foi atendido
            queue_remove((queue_t **)&disk.wait, (queue_t *)req->task);

            // set the task's status to ready
            req->task->state = 'R';

            // append the task to the ready queue
            queue_append((queue_t **)&readyQueue, (queue_t *)req->task);
            // task_resume((task_t *)req->task);
            queue_remove((queue_t **)&(disk.queue), (queue_t *)req);
            free(req);
            disk.sig = 0;
        }

        // se o disco estiver livre e houver pedidos de E/S na fila
        // int diskS = disk_cmd(DISK_CMD_STATUS, 0, 0);
        // printf("%d\n", diskS);
        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && &disk.queue)
        {
            cont++;
            // escolhe na fila o pedido a ser atendido, usando FCFS
            // FCFS(&disk.queue);
            req = disk.queue;
            // solicita ao disco a operação de E/S, usando disk_cmd()
            printf("%d\n", req->op);
            disk_cmd(req->op, req->block, req->buffer);
            printf("%d\n", cont);
        }

        if (disk.queue != NULL && disk.wait != NULL)
        {
            queue_remove((queue_t **)&readyQueue, (queue_t *)&tDisk);

            tDisk.state = 'S';
        }
        else
            tDisk.state = 'R';

        // libera o semáforo de acesso ao disco
        sem_up(&(disk.ac));
        // suspende a tarefa corrente (retorna ao dispatcher)
        task_yield();
    }
}

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init(int *numBlocks, int *blockSize)
{
    // registra a ação para o sinal de timer SIGUSR1
    actDisk.sa_handler = disk_handle;
    sigemptyset(&actDisk.sa_mask);
    actDisk.sa_flags = 0;
    if (sigaction(SIGUSR1, &actDisk, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // inicia disco
    if (disk_cmd(DISK_CMD_INIT, 0, 0) < 0)
        return -1;

    // get tamanho disco
    disk.numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    // get tamanho do bloco
    disk.blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    if (disk.numBlocks < 0 || disk.blockSize < 0)
        return -1;
    *numBlocks = disk.numBlocks;
    *blockSize = disk.blockSize;

    disk.queue = NULL;
    disk.wait = NULL;
    disk.sig = 0;
    sem_create((&disk.ac), 1);
    sem_down(&disk.ac);

    // cria a tarefa que vai gerenciar o disco
    task_create(&tDisk, diskDriverBody, NULL);
    task_resume(&tDisk);
    tDisk.state = 'S';

    sem_up(&disk.ac);
    printf("Disco criado\n");
    return 0;
}

// leitura de um bloco, do disco para o buffer
int disk_block_read(int block, void *buffer)
{
    if (block < 0 || !buffer)
        return -1;
    disk_request_t *request = create_request(block, buffer, DISK_CMD_READ);
    // obtém o semáforo de acesso ao disco
    sem_down(&disk.ac);
    // inclui o pedido na fila_disco
    queue_append((queue_t **)&disk.queue, (queue_t *)request);

    if (tDisk.state == 'S')
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        task_resume((task_t *)&tDisk);
    }
    // libera semáforo de acesso ao disco
    sem_up(&disk.ac);

    // suspende a tarefa corrente (retorna ao dispatcher)
    task_yield();
    return 0;
}
// escrita de um bloco, do buffer para o disco
int disk_block_write(int block, void *buffer)
{
    if (block < 0 || !buffer)
        return -1;
    disk_request_t *request = create_request(block, buffer, DISK_CMD_WRITE);
    // obtém o semáforo de acesso ao disco
    sem_down(&disk.ac);
    // inclui o pedido na fila_disco
    queue_append((queue_t **)&disk.queue, (queue_t *)request);
    if (tDisk.state == 'S')
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        task_resume((task_t *)&tDisk);
    }

    // libera semáforo de acesso ao disco
    sem_up(&disk.ac);

    // suspende a tarefa corrente (retorna ao dispatcher)
    task_yield();
    return 0;
}

// void FCFS(queue_t *dQueue)
// {
//     int seek_count = 0;
//     disk_request int cur_track, distance;

//     for (int i = 0; i < DISK_CMD_DISKSIZE; i++)
//     {
//         cur_track = disk;

//         // calculate absolute distance
//         distance = fabs(head - cur_track);

//         // increase the total count
//         seek_count += distance;

//         // accessed track is now new head
//         head = cur_track;
//     }
// }

// void scheduler_SSTF()
// {
//     disk_request_t *req = NULL;
//     while (1)
//     {
//         sem_down(&(disk.ac));
//         if (disk.sig == 1)
//         {
//             queue_remove((queue_t **)&disk.wait, (queue_t *)req->task);
//             req->task->state = 'R';
//             queue_append((queue_t **)&readyQueue, (queue_t *)req->task);
//             queue_remove((queue_t **)&disk.queue, (queue_t *)req);
//             disk.sig = 0;
//         }

//         if (disk_cmd(DISK_CMD_STATUS, 0, 0) == 1 && disk.queue != NULL)
//         {
//             int shortestSeek = 2147483647;
//             disk_request_t *shortestReq = NULL;

//             // Encontra o pedido com o menor tempo de busca
//             disk_request_t *currReq = disk.queue;
//             // while (currReq != NULL)
//             // {
//             //     int currSeek = abs(disk.currTrack - currReq->block);
//             //     if (currSeek < shortestSeek)
//             //     {
//             //         shortestSeek = currSeek;
//             //         shortestReq = currReq;
//             //     }
//             //     currReq = currReq->next;
//             // }

//             req = shortestReq;
//             disk_cmd(req->op, req->block, req->buffer);
//         }

//         if (disk.queue != NULL && disk.wait != NULL)
//         {
//             queue_remove((queue_t **)&readyQueue, (queue_t *)&tDisk);
//             tDisk.state = 'S';
//         }
//         else
//             tDisk.state = 'R';

//         sem_up(&(disk.ac));
//         task_yield();
//     }
// }
