#include "headers.h"
int sem1;
enum Status {
    Running,
    Stopped,
    Finished
};
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    short *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};
typedef struct Node Node;
struct Node
{
    Node *left;
    Node *right;
    int start;
    int end;
    int occupied;
    int size;
};
struct processData
{
    struct Node * allocationSpace;
    int memSize;
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int idTextFile;
    int waitingTime;
    int remTime;
    enum Status state;
    int finishedTime;
};

struct msgbuff
{
    long mtype;
    struct processData sentProcess;
};
void enqueue(struct processData insert_item, struct processData *inputProcesses, int lines);
struct processData dequeue(struct processData *inputProcesses, int lines);
void show();
int peekArrivalTime(struct processData *inputProcesses);

void clearResources(int);
int Rear = -1;
int Front = -1;
int msgq_id_s;
int upid;
int downid;
int main(int argc, char *argv[])
{
     union Semun semun;
    key_t key_id3;
    key_id3 = ftok("keyfile", 75);  //semaphore to synchronise clock between processes
    sem1 = semget(key_id3, 1, 0666 | IPC_CREAT);
    semun.val = 0;
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    signal(SIGINT, clearResources); //deletes ipcs
    int ch;
    FILE *myfile = fopen("processes.txt", "r");
    int lines = 0;
    while (!feof(myfile))
    {
        ch = fgetc(myfile);
        if (ch == '\n')
        {
            lines++;
        }
    }
    fclose(myfile);

    FILE *file = fopen("processes.txt", "r");
    struct processData inputProcesses[lines];
    char str[100];
    printf("lines = %d\n", lines);
    fgets(str, 100, file); // skips first line

    for (int i = 0; i < lines - 1; i++) //creates a process object for each process
    {
        struct processData newProcess;
        fscanf(file, "%d %d %d %d %d", &newProcess.id, &newProcess.arrivaltime, &newProcess.runningtime, &newProcess.priority, &newProcess.memSize);
        // printf("%d %d %d %d\n",newProcess.id,newProcess.arrivaltime,newProcess.runningtime,newProcess.priority);
        newProcess.remTime=newProcess.runningtime;
        newProcess.state=Stopped;
        newProcess.waitingTime=0;
        newProcess.idTextFile=newProcess.id;
        enqueue(newProcess, inputProcesses, lines);
    }
    show(inputProcesses);
    int schedulingType, quantum;
    printf("Enter your choice of scheduling algorithm : \n"); // user selects scheduling algorithm
    printf("Enter 1 for HPF, 2 for SRTN, 3 for RR: ");
    scanf("%d", &schedulingType);
    switch (schedulingType)
    {
    case 1:
        break;
    case 2:
        break;
    case 3:
        printf("Enter quantum for RR: ");
        scanf("%d", &quantum);
        break;
    default:
        printf("Incorrect choice \n");
    }
    printf("my quantum = %d \n", quantum);
    // TODO Initialization
    // 1. Read the input files.
    
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    int clkpid, stat_loc;
    clkpid = fork(); // creating instance of clock process

    if (clkpid == -1)
        perror("error in fork");

    else if (clkpid == 0)
    {
        execv("./clk.out", NULL);
    }
    initClk();

    // To get time use this

    int schedulerpid;
    schedulerpid = fork(); // creating instance of scheduler process

    key_t key_id;
    // UP Queue.
    key_id = ftok("keyfile", 65);
    msgq_id_s = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id_s == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    key_t key_idStoP;
    // UP Queue from scheduler to process
    key_idStoP = ftok("keyfile", 66);
    upid = msgget(key_idStoP, 0666 | IPC_CREAT);

    if (upid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    key_t ket_idProctoSch;
    // down Queue from process to scheduler
    ket_idProctoSch = ftok("keyfile", 67);
    downid = msgget(ket_idProctoSch, 0666 | IPC_CREAT);

    if (downid == -1)
    {
        perror("Error in create");
        exit(-1);
    }


    if (schedulerpid == -1)
        perror("error in fork");

    else if (schedulerpid == 0)
    {
        char typestr[2];
        char RRstr[2];
        char numOfProcesses[2];
        sprintf(typestr, "%d", schedulingType); // 1--> HPF, 2--> SRTN, 3--> RR
        sprintf(RRstr, "%d", quantum);
        sprintf(numOfProcesses,"%d",lines-1);
        char *args[] = {"scheduler", typestr, RRstr,numOfProcesses, NULL};
        execv("./scheduler.out", args);
    }
    while (1)
    {
        int currTime = getClk();
        int checkSend;
        int peekedArrivalTime = peekArrivalTime(inputProcesses);
        if (peekedArrivalTime != -1)
        {
            if (currTime == peekArrivalTime(inputProcesses)) // a new process must enter ready queue as it has now arrived
            {
                struct msgbuff sendmessage;
                sendmessage.sentProcess = inputProcesses[Front];
                sendmessage.mtype = 1;
                int checkSendQueues = msgsnd(msgq_id_s, &sendmessage, sizeof(sendmessage.sentProcess), !IPC_NOWAIT); // give scheduler the process
               // printf("sent from PG at %d\n",currTime);
                if (checkSendQueues == -1)
                    perror("Errror in sending from process generator");
                dequeue(inputProcesses, lines);
            }
        }
        //  printf("current time is %d\n", x);
    }

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void enqueue(struct processData insert_item, struct processData *inputProcesses, int lines)
{

    if (Rear == lines - 1)
        printf("Overflow \n");
    else
    {
        if (Front == -1)
            Front = 0;
        //  printf("Element to be inserted in the Queue\n : ");
        Rear = Rear + 1;
        inputProcesses[Rear] = insert_item;
    }
}

struct processData dequeue(struct processData *inputProcesses, int lines)
{
    if (Front == -1 || Front > Rear)
    {
        // printf("Underflow \n");
        return;
    }
    else
    {
        // printf("Element arrival time deleted from the Queue: %d\n", inputProcesses[Front].id);
        //   printf("Element Id deleted from the Queue: %d\n", inputProcesses[Front].arrivaltime);
        Front = Front + 1;
        return inputProcesses[Front - 1];
    }
}

int peekArrivalTime(struct processData *inputProcesses)
{
    if (Front == -1 || Front > Rear)
    {
        // printf("Underflow \n");
        return -1;
    }
    return inputProcesses[Front].arrivaltime;
}

void show(struct processData *inputProcesses)
{

    if (Front == -1)
        printf("Empty Queue \n");
    else
    {
        printf("Queue: \n");
        for (int i = Front; i <= Rear; i++)
        {
            printf("Id= %d ", inputProcesses[i].id);
            printf("arrival time= %d ", inputProcesses[i].arrivaltime);
            printf("running time= %d ", inputProcesses[i].runningtime);
            printf("priority= %d ", inputProcesses[i].priority);
            printf("\n");
        }
        printf("\n");
    }
}
void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    msgctl(msgq_id_s, IPC_RMID, (struct msqid_ds *)0);
    msgctl(upid, IPC_RMID, (struct msqid_ds *)0);
    msgctl(downid, IPC_RMID, (struct msqid_ds *)0);
    semctl(sem1, IPC_RMID, (struct semid_ds *)0);
    kill(getpid(), SIGKILL);
}
