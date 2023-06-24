// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef struct disk_request_t
{
  
  struct disk_request_t *prev;
  struct disk_request_t *next; // ponteiros para usar em fila 
  task_t *task; // pede o disco
  int op; // read ou write
  int block;
  void *buffer;
} disk_request_t;

// estrutura que representa um disco no sistema operacional
typedef struct disk_t
{
  // completar com os campos necessarios
  struct disk_request_t *queue; // fila responsável pela requisição dos discos
  semaphore_t ac; // semáforo que permite acesso ao disco
  task_t *wait; // tarefa responsável pelo estado de ocupado do disco
  int numBlocks; // qtd de blocos
  int blockSize; // tam do bloco
  int sig;
} disk_t;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
