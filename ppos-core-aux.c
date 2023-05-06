#include "ppos.h"
#include "ppos-core-globals.h"

// ****************************************************************************
// Coloque aqui as suas modificações, p.ex. includes, defines variáveis,
// estruturas e funções

// ****************************************************************************
#define DEF_TICKS 20;

// variáveis globais que auxiliam na contabilização de tempo da tarefa no processador
int auxInProc, auxEndProc;

// estrutura que define um tratador de sinal (deve ser global ou static)
static struct sigaction action;

// estrutura de inicialização to timer
static struct itimerval timer;

void trataTicks(int signum)
{
    systemTime++;
    if (taskExec != taskDisp)
    {
        if (taskExec->quantum >= 0)
            taskExec->quantum--;
        else
            task_yield();
    }
}

void before_ppos_init()
{

    // put your customization here

#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

void after_ppos_init()
{
    // put your customization here
    systemTime = 0;
    taskMain->quantum = DEF_TICKS;
    taskMain->tExec = systime();
    taskMain->act = 0;
    auxInProc = 0;
    auxEndProc = 0;
    // registra a ação para o sinal de timer SIGALRM
    action.sa_handler = trataTicks;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }
    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }

#ifdef DEBUG
    printf("\ninit - AFTER");
#endif
}

void before_task_create(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_create - BEFORE - [%d]", task->id);
#endif
}

void after_task_create(task_t *task)
{
    // put your customization here
    task->tExec = systime();
    if (task != taskDisp)
    {
        task->quantum = DEF_TICKS;
    }
#ifdef DEBUG
    printf("\ntask_create - AFTER - [%d]", task->id);
#endif
}

void before_task_exit()
{
    // put your customization here
    taskExec->tExec = systime() - taskExec->tExec;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
           taskExec->id, taskExec->tExec, taskExec->tProc, taskExec->act);
    if (countTasks <= 2)
    {
        printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
               taskDisp->id, taskExec->tExec, taskDisp->tProc, taskDisp->act);
    }

#ifdef DEBUG
    printf("\ntask_exit - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_exit()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - AFTER- [%d]", taskExec->id);
#endif
}

void before_task_switch(task_t *task)
{
    // put your customization here
    auxEndProc = systime();
    taskExec->tProc += auxEndProc - auxInProc;
    taskExec->act++;
    if (task != NULL)
    {
        task->quantum = DEF_TICKS;
    }
#ifdef DEBUG
    printf("\ntask_switch - BEFORE - [%d -> %d]", taskExec->id, task->id);
#endif
}

void after_task_switch(task_t *task)
{
    // put your customization here
    auxInProc = systime();
#ifdef DEBUG
    printf("\ntask_switch - AFTER - [%d -> %d]", taskExec->id, task->id);
#endif
}

void before_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - BEFORE - [%d]", taskExec->id);
#endif
}
void after_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - AFTER - [%d]", taskExec->id);
#endif
}

void before_task_suspend(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - BEFORE - [%d]", task->id);
#endif
}

void after_task_suspend(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - AFTER - [%d]", task->id);
#endif
}

void before_task_resume(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - BEFORE - [%d]", task->id);
#endif
}

void after_task_resume(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - AFTER - [%d]", task->id);
#endif
}

void before_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - AFTER - [%d]", taskExec->id);
#endif
}

int before_task_join(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_task_join(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_create(semaphore_t *s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_create(semaphore_t *s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_down(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_down(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_up(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_up(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_destroy(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_destroy(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_create(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_create(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_lock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_lock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_unlock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_unlock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_destroy(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_destroy(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_create(barrier_t *b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_create(barrier_t *b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_join(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_join(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_destroy(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_destroy(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_create(mqueue_t *queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_create(mqueue_t *queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_send(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_send(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_recv(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_recv(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_destroy(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_destroy(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_msgs(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_msgs(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

task_t *scheduler()
{
    task_t *taskAux, *minPrio;
    taskAux = readyQueue;
    minPrio = taskAux;
    do
    {
        taskAux = taskAux->next;
        if (taskAux->din < minPrio->din)
            minPrio = taskAux;
        else if (taskAux->din == minPrio->din)
            if (taskAux->est < minPrio->est)
                minPrio = taskAux;
    } while (taskAux != readyQueue);
    taskAux = readyQueue;
    do
    {
        if (taskAux->din > -20)
            taskAux->din--;
        taskAux = taskAux->next;
    } while (taskAux != readyQueue);

    minPrio->din = minPrio->est;
    return minPrio;
}

void task_setprio(task_t *task, int prio)
{
    if (task != NULL)
    {
        task->din = prio;
        task->est = prio;
    }
    else
    {
        taskExec->din = prio;
        taskExec->est = prio;
    }
}

int task_getprio(task_t *task)
{
    if (task != NULL)
        return task->est;
    return taskExec->est;
}