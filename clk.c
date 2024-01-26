/*
 * This file is done for you.
 * Probably you will not need to change anything.
 * This file represents an emulated clock for simulation purpose only.
 * It is not a real part of operating system!
 */

#include "headers.h"
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    short *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};
int sem1;
 union Semun semun;

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

int up(int sem)
{
    int x=semctl(sem1,0, GETVAL, semun);
    if(x==1)
    {
        return 0;
    }
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
    return 1;
}
int shmid;

/* Clear the resources before exit */
void cleanup(int signum)
{
    shmctl(shmid, IPC_RMID, NULL);
    printf("Clock Terminating!\n");
    exit(0);
}

/* This file represents the system clock for ease of calculations */
int main(int argc, char * argv[])
{
   
     key_t key_id3;
     key_id3 = ftok("keyfile", 75);
       sem1 = semget(key_id3, 1, 0666 | IPC_CREAT);
    printf("Clock Starting!\n");
    signal(SIGINT, cleanup);
    int clk = 0;
    //Create shared memory for one integer variable 4 bytes
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    if ((long)shmid == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int * shmaddr = (int *) shmat(shmid, (void *)0, 0);
    if ((long)shmaddr == -1)
    {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    *shmaddr = clk; /* initialize shared memory */
    while (1)
    {
        
       int x=up(sem1);
     sleep(1);
     if(x==1)
     {

     
        (*shmaddr)++;
     }
    }
}

