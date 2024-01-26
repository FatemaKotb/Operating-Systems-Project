
#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

struct msgbuffToProcess
{
    long mtype;
    int runningtime;
};

int main(int agrc, char *argv[])
{
    initClk();

    int upid;
    key_t key_idStoP;
    // UP Queue from scheduler to process
    key_idStoP = ftok("keyfile", 66);
    upid = msgget(key_idStoP, 0666 | IPC_CREAT);

    if (upid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    int downid;
    key_t ket_idProctoSch;
    // UP Queue from scheduler to process
    ket_idProctoSch = ftok("keyfile", 67);
    downid = msgget(ket_idProctoSch, 0666 | IPC_CREAT);

    if (downid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    struct msgbuffToProcess recMessage;
    int rec_val = msgrcv(upid, &recMessage, sizeof(recMessage.runningtime), getpid(), !IPC_NOWAIT);
    // Failed to receive message.
    if (rec_val == -1)
    {
        perror("Error in receive");
    }
    remainingtime = recMessage.runningtime;
    remainingtime--;
    //printf("clock=%d i am process %d and have %d seconds remaining\n",getClk()-1,getpid(),remainingtime);

    while (remainingtime > 0)
    {
        int rec_val = msgrcv(upid, &recMessage, sizeof(recMessage.runningtime), getpid(), !IPC_NOWAIT);
        // Failed to receive message.
        if (rec_val == -1)
        {
            perror("Error in receive");
        }
        else
        {
            remainingtime--;//currently running process decrements itself upon msgrcv
          //  printf("clock=%d i am process %d and have %d seconds remaining\n",getClk()-1,getpid(),remainingtime);
        }
    }

    struct msgbuffToProcess sendMessage;
    sendMessage.mtype = getpid();
    int checkSend = msgsnd(downid, &sendMessage, sizeof(sendMessage.runningtime), !IPC_NOWAIT); //when process is done sends to scheduler and terminates

    destroyClk(false);
    exit(0);
    return 0;
}
