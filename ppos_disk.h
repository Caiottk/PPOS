// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>

#include "ppos.h"
#include "ppos-core-globals.h"
#include "disk.h"

#define FCFS 0
#define SSTF 1
#define CSCAN 2
#define INT_MAX 2147483647

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef struct disk_request_t
{
  
  struct disk_request_t *prev;
  struct disk_request_t *next; // ponteiros para usar em fila

  int op; // read ou write
  int block;
  void *buffer;
} disk_request_t;

// estrutura que representa um disco no sistema operacional
typedef struct disk_t
{
  // completar com os campos necessarios
  // estruturas para auxiliar os estados do disco e coordenar a parte crítica
  disk_request_t req; // Fila de requisições de leitura/escrita do disco
  mutex_t mreq; // Mutex para controle do acesso à fila de requisições
  mutex_t mqueue; // Mutex para controle do acesso à fila de discos
  semaphore_t vazio; // Semáforo que indica se a fila de requisições está vazia
  semaphore_t cheio; // Semáforo que indica se a fila de requisições está cheia
  task_t task; //Tarefa que executa o gerenciador de disco

  // atributos de um disco 
  int numBlocks; // Número de blocos do disco
  int blockSize; // Tamanho de cada bloco do disco em bytes
  int block; // Número do bloco atual do disco
  void* buffer; // Buffer utilizado para leitura/escrita de blocos
  int tIn; // Tempo de início da execução de uma requisição
  int qtdBlocosPer; // Quantidade de blocos percorridos
  int tExec; // Tempo total de execução do disco.

  // flags para coodernar requisições
  char init; // Flag que indica se o disco foi inicializado
  int state; // Estado atual do disco
  short pacotes; // Contador para coordenação da comunicação entre os processos de requisições pendentes na fila do disco
  int sched; //  Flag do escalonador
} disk_t;

// disco
disk_t disk;
//fila de discos
disk_request_t* disk_queue;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: numBlocksanho do disco, em blocos
// blockSize: numBlocksanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
