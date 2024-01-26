#include "headers.h"

float simEndTime;
float simStartTime;
float usefultime = 0;
float wtatotal = 0;
float waitingtotal = 0;
int Processes;
int index = 0;
FILE *fileobj;
FILE *fileobj2;
FILE *fileobj3;

enum Status
{
    Running,
    Stopped,
    Finished
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
    struct Node *allocationSpace;
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

typedef struct
{
    struct processData *processes;
    int front;
    int rear;
    int maxSize;
} ProcessQueue;

struct Node *createNode(int newSize, int startMem, int endMem)
{
    struct Node *newNode = malloc(sizeof(struct Node));
    newNode->size = newSize;
    newNode->occupied = 0; // not occupied
    newNode->start = startMem;
    newNode->end = endMem;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}
void searchBestFit(struct Node *root, int requestSize, struct Node **bestNode)
{
    if (root->left == NULL && root->right == NULL) // leaf found
    {
        if (root->size >= requestSize && root->occupied == 0) // fits the request and is availble
        {
            if (*bestNode == NULL) // for first assign
            {
                *bestNode = root;
            }
            else
            {
                if (root->size < (*bestNode)->size) // better fit than the current
                {
                    *bestNode = root;
                }
            }
        }
    }
    else
    {
        searchBestFit(root->left, requestSize, bestNode);
        searchBestFit(root->right, requestSize, bestNode);
    }
}
int allocateSpace(struct Node *bestFit, int requestSize, struct Node **forProcess)
{
    if ((bestFit->size) / 2 < requestSize)
    {
        bestFit->occupied = 1;
        *forProcess = bestFit;
        return 1;
    }
    // General case

    bestFit->left = createNode((bestFit->size) / 2, bestFit->start, bestFit->start + ((bestFit->size) / 2) - 1);
    bestFit->right = createNode((bestFit->size) / 2, (bestFit->end) - ((bestFit->size) / 2) + 1, bestFit->end);
    int isAllocated = 0;
    isAllocated = allocateSpace(bestFit->left, requestSize, forProcess);
    return isAllocated;
}
void mergeSlots(struct Node *root)
{
    if (root == NULL || root->left == NULL && root->right == NULL)
    {
        return;
    }
    mergeSlots(root->left);
    mergeSlots(root->right);
    if (root->right->occupied == 0 && root->left->occupied == 0 && root->left->left == NULL && root->left->right == NULL && root->right->left == NULL && root->right->right == NULL) // both children are not occupied
    {
        free(root->right);
        free(root->left);
        root->left = NULL;
        root->right = NULL;
    }
}
void freeAllocatedResource(struct Node **toDelete, struct Node *root)
{
    (*toDelete)->occupied = 0;
    mergeSlots(root);
}
void printLeaves(struct Node *root)
{
    if (root->left == NULL && root->right == NULL) // leaf found
    {
        printf("Slot start= %d\n", root->start);
        printf("Slot end= %d\n", root->end);
        printf("slot size= %d\n", root->size);
        printf("slot occupied = %d\n", root->occupied);
        printf("------------------------------------------\n");
        return;
    }
    printLeaves(root->left);
    printLeaves(root->right);
}
// ******************Auxiliary Functions***********************

// Create a node in tree
// Function to initialize the ProcessQueue
void initialize(ProcessQueue *q, int maxSize)
{
    // malloc: It takes one argument, which is the size of the memory you want to allocate in bytes, and returns a pointer to the allocated memory.
    q->processes = (struct processData *)malloc(maxSize * sizeof(struct processData));
    q->front = -1; // front is initialized to -1
    q->rear = -1;  // rear is initialized to -1
    q->maxSize = maxSize;
}

// Function to add a process to the ProcessQueue
void enqueue(ProcessQueue *q, struct processData p)
{
    // If the rear of the queue has reached the maximum size, the queue is full
    if (q->rear == q->maxSize - 1)
    {
        printf("Overflow \n");
    }
    else
    {
        // If the queue is empty, set the front to 0
        if (q->front == -1)
            q->front = 0;
        // Add the process to the next available spot in the queue
        q->rear = q->rear + 1;
        // printf("\nRear at enqueue of %d= %d\n",p.runningtime,q->rear);
        q->processes[q->rear] = p;
    }
}

// Function to remove a process from the ProcessQueue
struct processData dequeue(ProcessQueue *q)
{
    // If the front of the queue is -1, the queue is empty
    if (q->front == -1)
    {
        // printf("Queue Dequeue Underflow \n");
        struct processData p = {-1, -1, -1, -1};
        p.id = -1;
        return p;
    }
    else
    {
        // Remove the process from the front of the queue
        struct processData removedProcess = q->processes[q->front];
        // Shift the remaining processes
        for (int j = q->front; j < q->rear; j++)
        {
            q->processes[j] = q->processes[j + 1];
        }
        q->rear--;
        // If the queue becomes empty after removal, set front and rear to -1
        if (q->front > q->rear)
            q->front = q->rear = -1;
        return removedProcess;
    }
}

// Function to get the process at the front of the ProcessQueue
struct processData peek(ProcessQueue *q)
{
    // If the front of the queue is -1, the queue is empty
    if (q->front == -1)
    {
        // printf("Queue Peek Underflow \n");
        struct processData p = {-1, -1, -1, -1};
        p.id = -1;
        return p;
    }
    // Return the process at the front of the queue
    return q->processes[q->front];
}

// Function to display the processes in the ProcessQueue
void print(ProcessQueue *q)
{
    // If the front of the queue is -1, the queue is empty
    // if (q->front == -1)
    //     printf("Empty Queue \n");
    // else
    if (q->front != -1)
    {
        printf("Queue: \n");
        // Loop through the queue from front to rear and print each process
        for (int i = q->front; i <= q->rear; i++)
        {
            printf("Id              =   %d\n", q->processes[i].id);
            printf("arrival time    =   %d\n", q->processes[i].arrivaltime);
            printf("running time    =   %d\n", q->processes[i].runningtime);
            printf("priority        =   %d\n", q->processes[i].priority);
            printf("\n");
        }
        printf("\n");
    }
}

// Function to chevk if the processQueue is empty.
int isEmpty(ProcessQueue *q)
{
    return (q->front == -1);
}
struct processData dequeueByID(ProcessQueue *q, int id)
{
    // If the queue is empty
    if (q->front == -1)
    {
        // printf("Queue DequeueByID Underflow \n");
        struct processData p = {-1, -1, -1, -1};
        return p;
    }
    else
    {
        // Search for the process with the given ID
        for (int i = q->front; i <= q->rear; i++)
        {
            if (q->processes[i].id == id)
            {
                // If the process is found, remove it and shift the remaining processes
                struct processData removedProcess = q->processes[i];
                for (int j = i; j < q->rear; j++)
                {
                    q->processes[j] = q->processes[j + 1];
                }
                q->rear--;
                // If the queue becomes empty after removal, set front and rear to -1
                if (q->front > q->rear)
                    q->front = q->rear = -1;
                //  printf("\nRear at dequeue of %d= %d\n",removedProcess.runningtime,q->rear);
                return removedProcess;
            }
        }
        // If the process is not found, return a default process
        printf("Process not found \n");
        struct processData p = {-1, -1, -1, -1};
        return p;
    }
}
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    short *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

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

struct msgbuff
{
    long mtype;
    struct processData sentProcess;
};

struct msgbuffToProcess
{
    long mtype;
    int runningtime;
};

int size = 0;
void swap(struct processData *a, struct processData *b)
{
    struct processData temp = *b;
    *b = *a;
    *a = temp;
}

// Function to heapify the tree
void heapify(struct processData array[], int size, int i)
{
    if (size == 1)
    {
        printf("Single element in the heap");
    }
    else
    {
        // Find the largest among root, left child and right child
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < size)
        {
            if (array[l].priority < array[largest].priority)
            {
                largest = l;
            }
            if (array[l].priority == array[largest].priority)
            {
                if (array[l].idTextFile < array[largest].idTextFile)
                {
                    largest = l;
                }
            }
        }
        if (r < size)
        {
            if (array[r].priority < array[largest].priority)
            {
                largest = r;
            }
            if (array[r].priority == array[largest].priority)
            {
                if (array[r].idTextFile < array[largest].idTextFile)
                {
                    largest = r;
                }
            }
        }
        // Swap and continue heapifying if root is not largest
        if (largest != i)
        {
            swap(&array[i], &array[largest]);
            heapify(array, size, largest);
        }
    }
}

void heapify_srtn(struct processData array[], int size, int i)
{
    if (size == 1)
    {
        printf("Single element in the heap");
    }
    else
    {
        // Find the largest among root, left child and right child
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < size)
        {
            if (array[l].runningtime < array[largest].runningtime)
            {
                largest = l;
            }
            if (array[l].runningtime == array[largest].runningtime)
            {
                if (array[l].idTextFile < array[largest].idTextFile)
                {
                    largest = l;
                }
            }
        }
        if (r < size)
        {
            if (array[r].runningtime < array[largest].runningtime)
            {
                largest = r;
            }
            if (array[r].runningtime == array[largest].runningtime)
            {
                if (array[r].idTextFile < array[largest].idTextFile)
                {
                    largest = r;
                }
            }
        }

        // Swap and continue heapifying if root is not largest
        if (largest != i)
        {
            swap(&array[i], &array[largest]);
            heapify(array, size, largest);
        }
    }
}

// Function to insert an element into the tree
void insert(struct processData array[], struct processData newNum)
{
    if (size == 0)
    {
        array[0] = newNum;
        size += 1;
    }
    else
    {
        array[size] = newNum;
        size += 1;
        for (int i = size / 2 - 1; i >= 0; i--)
        {
            heapify(array, size, i);
        }
    }
}
void insert_srtn(struct processData array[], struct processData newNum)
{
    if (size == 0)
    {
        array[0] = newNum;
        size += 1;
    }
    else
    {
        array[size] = newNum;
        size += 1;
        for (int i = size / 2 - 1; i >= 0; i--)
        {
            heapify_srtn(array, size, i);
        }
    }
}
int peekRootId(struct processData array[])
{
    if (size == 0)
    {
        return -1;
    }
    return array[0].id; // returns pid of maximum priority
}
struct processData peekRoot(struct processData array[])
{
    return array[0];
}
int peekRootRunningTime(struct processData array[])
{
    if (size == 0)
    {
        return -1;
    }
    return array[0].runningtime; // returns pid of maximum priority
}
// Function to delete an element from the tree
void deleteRoot(struct processData array[])
{
    swap(&array[0], &array[size - 1]);
    size -= 1;
    for (int i = size / 2 - 1; i >= 0; i--)
    {
        heapify(array, size, i);
    }
}

// Function to delete an element from the tree given pos
struct processData deleteProcessGivenID(struct processData array[], int processID)
{
    struct processData deletedProcess;
    int i;
    for (i = 0; i < size; i++)
    {
        if (processID == array[i].id)
        {
            deletedProcess = array[i];
            break;
        }
    }

    swap(&array[i], &array[size - 1]);
    size -= 1;
    for (int i = size / 2 - 1; i >= 0; i--)
    {
        heapify(array, size, i);
    }
    return deletedProcess;
}

struct processData deleteProcessGivenID_SRTN(struct processData array[], int processID)
{
    struct processData deletedProcess;
    int i;
    for (i = 0; i < size; i++)
    {
        if (processID == array[i].id)
        {
            deletedProcess = array[i];
            break;
        }
    }

    swap(&array[i], &array[size - 1]);
    size -= 1;
    for (int i = size / 2 - 1; i >= 0; i--)
    {
        heapify_srtn(array, size, i);
    }
    return deletedProcess;
}

// Print the array
void printArray(struct processData array[], int size)
{
    for (int i = 0; i < size; ++i)
        printf("[%d , %d]", array[i].id, array[i].runningtime);
    printf("\n");
}

void adjustPCB(struct processData PCB[], int processNum, int runningProcessID, int prevID, float WTAvalues[])
{
    usefultime++;
    // printf("usefultime= %f\n", usefultime);
    struct processData prevProcess;
    if (prevID == -1)
    {
        prevProcess.state = Finished;
        prevProcess.id = -1;
    }
    else
    {
        for (int i = 0; i < processNum; i++)
        {
            if (PCB[i].id == prevID)
            {
                prevProcess = PCB[i];
                break;
            }
        }
    }
    for (int i = 0; i < processNum; i++)
    {

        if (PCB[i].id != runningProcessID && PCB[i].state != Finished)
        {
            PCB[i].waitingTime++;
            PCB[i].state = Stopped;
        }
        if (PCB[i].id == runningProcessID)
        {
            PCB[i].state = Running;
            PCB[i].remTime--;
            if (PCB[i].remTime == 0) // Finished Process
            {
                PCB[i].state = Finished;
                PCB[i].finishedTime = getClk() + 1;
                int TAtime = PCB[i].finishedTime - PCB[i].arrivaltime;
                float WTAtime = (float)TAtime / (float)PCB[i].runningtime;
                if (PCB[i].runningtime == 1)
                {
                    if (PCB[i].id != prevProcess.id && prevProcess.state != Finished)
                    {
                        fprintf(fileobj, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), prevProcess.idTextFile, prevProcess.arrivaltime, prevProcess.runningtime, prevProcess.remTime, prevProcess.waitingTime);
                    }
                    fprintf(fileobj, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), PCB[i].idTextFile, PCB[i].arrivaltime, PCB[i].runningtime, PCB[i].remTime + 1, PCB[i].waitingTime);
                }
                if (PCB[i].runningtime != 1 && PCB[i].id != prevProcess.id)
                {
                    if (PCB[i].id != prevProcess.id && prevProcess.state != Finished)
                    {
                        fprintf(fileobj, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), prevProcess.idTextFile, prevProcess.arrivaltime, prevProcess.runningtime, prevProcess.remTime, prevProcess.waitingTime);
                    }
                    fprintf(fileobj, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), PCB[i].idTextFile, PCB[i].arrivaltime, PCB[i].runningtime, PCB[i].remTime + 1, PCB[i].waitingTime);
                }
                wtatotal = wtatotal + WTAtime;
                waitingtotal = waitingtotal + PCB[i].waitingTime;
                WTAvalues[index] = WTAtime;
                index++;
                fprintf(fileobj, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", PCB[i].finishedTime, PCB[i].idTextFile, PCB[i].arrivaltime, PCB[i].runningtime, PCB[i].remTime, PCB[i].waitingTime, TAtime, WTAtime);
            }
            else // process still not finished
            {
                if ((PCB[i].remTime + 1) == PCB[i].runningtime) // 1st time to start
                {
                    if (PCB[i].id != prevProcess.id && prevProcess.state != Finished)
                    {
                        fprintf(fileobj, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), prevProcess.idTextFile, prevProcess.arrivaltime, prevProcess.runningtime, prevProcess.remTime, prevProcess.waitingTime);
                    }
                    fprintf(fileobj, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), PCB[i].idTextFile, PCB[i].arrivaltime, PCB[i].runningtime, PCB[i].remTime + 1, PCB[i].waitingTime);
                }
                else // not its first time
                {
                    if (prevProcess.id == PCB[i].id)
                    {
                        // Do nothing i.e same process running
                    }
                    else // print stopped process(prev) and resume the current process
                    {
                        if (PCB[i].id != prevProcess.id && prevProcess.state != Finished)
                        {
                            fprintf(fileobj, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), prevProcess.idTextFile, prevProcess.arrivaltime, prevProcess.runningtime, prevProcess.remTime, prevProcess.waitingTime);
                        }
                        fprintf(fileobj, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), PCB[i].idTextFile, PCB[i].arrivaltime, PCB[i].runningtime, PCB[i].remTime + 1, PCB[i].waitingTime);
                    }
                }
            }
        }
    }
}
void schedulerPerf(float WTAvalues[]) // get statistics such as std deviation,utilisation ,avg wait time
{
    float stdtemp = 0;
    float utilisation = usefultime / (simEndTime);
    float wtaavg = wtatotal / (float)Processes;
    float std;
    for (int i = 0; i < Processes; i++)
    {
        stdtemp = stdtemp + pow((double)(WTAvalues[i] - wtaavg), 2.0);
    }
    std = sqrt((double)(stdtemp / (float)Processes));
    fprintf(fileobj2, "cpu utilisation= %.2f %% \n", utilisation * 100);
    fprintf(fileobj2, "Avg WTA= %.2f\n", wtatotal / (float)Processes);
    fprintf(fileobj2, "Avg waiting= %.2f\n", waitingtotal / (float)Processes);
    fprintf(fileobj2, "standard deviation= %.2f\n", std);
}

int main(int argc, char *argv[])
{

    fileobj = fopen("log.txt", "w"); // log files that statistics are written to
    fileobj2 = fopen("perf.txt", "w");
    fileobj3 = fopen("memory.txt", "w");
    if (fileobj == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    if (fileobj2 == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    if (fileobj3 == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    union Semun semun;
    key_t key_id3;
    key_id3 = ftok("keyfile", 75);
    int sem1 = semget(key_id3, 1, 0666 | IPC_CREAT); // semaphore to synch clocks
    semun.val = 0;
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    initClk();
    int flag = 0;
    int incProcressId;
    Processes = atoi(argv[3]);
    float WTAvalues[Processes];
    struct Node *root = createNode(1024, 0, 1023); // root for buddy system
    int terminatingcondition = Processes;          // numOfProcesses
    // printf("Num of processes: %d\n", atoi(argv[3]));
    // printf("the scheduling type= %s\n", argv[1]);
    // printf("the RR quantum= %s\n", argv[2]);
    struct processData readyHPFQueue[Processes];
    struct processData readySRTNQueue[Processes];
    ProcessQueue readyRRQueue;
    initialize(&readyRRQueue, Processes);
    ProcessQueue waitingList;
    initialize(&waitingList, Processes);
    // printf("readyqueue at first\n");
    print(&readyRRQueue);
    struct processData PCB[Processes]; // stores all processes and their data
    key_t key_id;
    int msgq_id_s;
    // UP Queue.
    key_id = ftok("keyfile", 65);
    msgq_id_s = msgget(key_id, 0666 | IPC_CREAT);
    if (msgq_id_s == -1)
    {
        perror("Error in create");
        exit(-1);
    }

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

    // printf("the scheduling type= %d\n", recmessage.mtype);
    // TODO implement the scheduler :)
    // upon termination release the clock resources
    int PCBindex = 0;
    int prevTempProcessID = -1;
    int prevProcessID = -1;
    int prevTime = getClk();

    int firstprocess = 0;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////HPF/////////////////////////////////////////////////////////////
    if (atoi(argv[1]) == 1) // highest priority first
    {

        while (1)
        {
            if (terminatingcondition == 0) // finished condition
            {
                simEndTime = getClk();
                schedulerPerf(&WTAvalues);
                break;
            }
            down(sem1);

            struct msgbuff recMessage;
            while (msgrcv(msgq_id_s, &recMessage, sizeof(recMessage.sentProcess), 0, IPC_NOWAIT) > 0) // try to receive a process
            {
                // Check for allocation space
                struct Node *bestFit = NULL;
                searchBestFit(root, recMessage.sentProcess.memSize, &bestFit);
                if (bestFit == NULL)
                {
                    // put in waiting list
                    enqueue(&waitingList, recMessage.sentProcess);
                }
                else
                {
                    struct Node *forProcesstemp = NULL;
                    allocateSpace(bestFit, recMessage.sentProcess.memSize, &forProcesstemp);
                    recMessage.sentProcess.allocationSpace = forProcesstemp;
                    fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), recMessage.sentProcess.memSize, recMessage.sentProcess.idTextFile, forProcesstemp->start, forProcesstemp->end);
                    int newPid = fork();

                    if (newPid != 0)
                    {
                        recMessage.sentProcess.id = newPid;
                        if (atoi(argv[1]) == 1) // HPF
                        {
                            PCB[PCBindex] = recMessage.sentProcess;
                            PCBindex++;
                            if (firstprocess == 0)
                            {
                                firstprocess = 1;
                                simStartTime = recMessage.sentProcess.arrivaltime;
                                // printf(" arrival time=%d\n",simStartTime);
                            }
                            insert(readyHPFQueue, recMessage.sentProcess); // inserted in readyqueue
                                                                           //  printf("After Each Insertion: \n");
                                                                           // printArray(readyHPFQueue, size);
                        }
                    }
                    else
                    {
                        execv("./process.out", NULL);
                    }
                }
            }
            {
                if (flag == 0 || incProcressId == -1)
                {
                    incProcressId = peekRootId(readyHPFQueue);
                    prevTempProcessID = incProcressId;
                    flag = 1;
                }

                if (incProcressId != -1)
                {

                    struct msgbuffToProcess recMessageFromProcess;
                    int rec_valFromProcess = msgrcv(downid, &recMessageFromProcess, sizeof(recMessageFromProcess.runningtime), 0, IPC_NOWAIT);
                    // Failed to receive message.
                    if (rec_valFromProcess == -1)
                    {
                        // perror("Error in receive");
                    }
                    else
                    {

                        int processIdToKill = recMessageFromProcess.mtype;
                        struct processData deletedProcess = deleteProcessGivenID(readyHPFQueue, processIdToKill);
                        fprintf(fileobj3, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), deletedProcess.memSize, deletedProcess.idTextFile, deletedProcess.allocationSpace->start, deletedProcess.allocationSpace->end);
                        freeAllocatedResource(&deletedProcess.allocationSpace, root);
                        while(1)
                        {

                        
                        struct processData checkedWaitingProcess = peek(&waitingList);
                        struct Node *checkBestFit = NULL;
                        if (checkedWaitingProcess.id != -1)
                        {
                            searchBestFit(root, checkedWaitingProcess.memSize, &checkBestFit);
                        }
                        if (checkedWaitingProcess.id == -1)
                        {
                          break;
                        }
                         if (checkBestFit == NULL)
                        {
                            break;
                        }
                        if (checkBestFit != NULL)
                        {
                            dequeue(&waitingList);
                            struct Node *toProcesstemp = NULL;
                            allocateSpace(checkBestFit, checkedWaitingProcess.memSize, &toProcesstemp);
                            checkedWaitingProcess.allocationSpace = toProcesstemp;
                            /////////////////////////////////////////////////////////////////////////////////////
                            int newPid = fork();

                            if (newPid != 0)
                            {
                                checkedWaitingProcess.id = newPid;
                                if (atoi(argv[1]) == 1) // HPF
                                {
                                    checkedWaitingProcess.waitingTime = getClk() - checkedWaitingProcess.arrivaltime;
                                    PCB[PCBindex] = checkedWaitingProcess;
                                    PCBindex++;
                                    insert(readyHPFQueue, checkedWaitingProcess); // inserted in readyqueue
                                }
                            }
                            else
                            {
                                execv("./process.out", NULL);
                            }
                            fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), checkedWaitingProcess.memSize, checkedWaitingProcess.idTextFile, toProcesstemp->start, toProcesstemp->end);
                        }
                        }
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        terminatingcondition--;

                        incProcressId = peekRootId(readyHPFQueue);
                        prevTempProcessID = incProcressId;
                        // printf("After Deletion: \n");
                        // printArray(readyHPFQueue, size);
                    }

                    int incProcessRunningTime = peekRootRunningTime(readyHPFQueue);
                    struct msgbuffToProcess sendMessage;
                    sendMessage.mtype = incProcressId;
                    sendMessage.runningtime = incProcessRunningTime;
                    if (incProcessRunningTime != -1)
                    {
                        // printf("clock=%d i am process %d \n", getClk(), incProcressId);
                        adjustPCB(&PCB, Processes, incProcressId, prevProcessID, &WTAvalues);
                        int checkSend = msgsnd(upid, &sendMessage, sizeof(sendMessage.runningtime), !IPC_NOWAIT); // tells the process to run
                        // Failed to receive message.
                        if (checkSend == -1)
                        {
                            perror("Error in send");
                        }
                        prevProcessID = prevTempProcessID;
                    }
                }
            }
        }
    }
    if (atoi(argv[1]) == 2) // shortest remaining time next
    {
        while (1)
        {
            if (terminatingcondition == 0)
            {
                simEndTime = getClk();
                schedulerPerf(&WTAvalues);
                break;
            }
            down(sem1);
            struct msgbuff recMessage;

            // Failed to receive message.

            while (msgrcv(msgq_id_s, &recMessage, sizeof(recMessage.sentProcess), 0, IPC_NOWAIT) > 0) // try to receive a process
            {

                // Check for allocation space
                struct Node *bestFit = NULL;
                searchBestFit(root, recMessage.sentProcess.memSize, &bestFit);
                if (bestFit == NULL)
                {
                    // put in waiting list
                    enqueue(&waitingList, recMessage.sentProcess);
                }
                else
                {
                    struct Node *forProcesstemp = NULL;
                    allocateSpace(bestFit, recMessage.sentProcess.memSize, &forProcesstemp);
                    recMessage.sentProcess.allocationSpace = forProcesstemp;
                    fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), recMessage.sentProcess.memSize, recMessage.sentProcess.idTextFile, forProcesstemp->start, forProcesstemp->end);
                    int newPid = fork();
                    if (newPid != 0)
                    {
                        recMessage.sentProcess.id = newPid;
                        if (atoi(argv[1]) == 2) // SRTN
                        {
                            PCB[PCBindex] = recMessage.sentProcess;
                            PCBindex++;
                            if (firstprocess == 0)
                            {
                                firstprocess = 1;
                                simStartTime = recMessage.sentProcess.arrivaltime;
                                // printf(" arrival time=%d\n",simStartTime);
                            }
                            insert_srtn(readySRTNQueue, recMessage.sentProcess); // inserted in readyqueue
                                                                                 // printf("After Each Insertion: \n");
                                                                                 // printArray(readySRTNQueue, size);
                        }
                    }
                    else
                    {
                        execv("./process.out", NULL);
                    }
                }
            }

            {

                int incProcressId = peekRootId(readySRTNQueue);
                prevTempProcessID = incProcressId;
                if (incProcressId != -1)
                {

                    struct msgbuffToProcess recMessageFromProcess;
                    int rec_valFromProcess = msgrcv(downid, &recMessageFromProcess, sizeof(recMessageFromProcess.runningtime), 0, IPC_NOWAIT);
                    // Failed to receive message.
                    if (rec_valFromProcess == -1)
                    {
                        // perror("Error in receive");
                    }
                    else
                    {
                        int processIdToKill = recMessageFromProcess.mtype;
                        struct processData deletedProcess = deleteProcessGivenID_SRTN(readySRTNQueue, processIdToKill);

                        fprintf(fileobj3, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), deletedProcess.memSize, deletedProcess.idTextFile, deletedProcess.allocationSpace->start, deletedProcess.allocationSpace->end);
                        freeAllocatedResource(&deletedProcess.allocationSpace, root);
                    while(1)
                    {

                    
                        struct processData checkedWaitingProcess = peek(&waitingList);
                        struct Node *checkBestFit = NULL;
                        if (checkedWaitingProcess.id != -1)
                        {
                            searchBestFit(root, checkedWaitingProcess.memSize, &checkBestFit);
                        }
                         if (checkedWaitingProcess.id == -1)
                        {
                            break;
                        }

                         if (checkBestFit == NULL)
                        {
                            break;
                        }

                        if (checkBestFit != NULL)
                        {
                            dequeue(&waitingList);
                            struct Node *toProcesstemp = NULL;
                            allocateSpace(checkBestFit, checkedWaitingProcess.memSize, &toProcesstemp);
                            checkedWaitingProcess.allocationSpace = toProcesstemp;
                            /////////////////////////////////////////////////////////////////////////////////////
                            int newPid = fork();

                            if (newPid != 0)
                            {
                                checkedWaitingProcess.id = newPid;
                                if (atoi(argv[1]) == 2) // SRTN
                                {
                                    checkedWaitingProcess.waitingTime = getClk() - checkedWaitingProcess.arrivaltime;
                                    PCB[PCBindex] = checkedWaitingProcess;
                                    PCBindex++;
                                    insert_srtn(readySRTNQueue, checkedWaitingProcess); // inserted in readyqueue
                                }
                            }
                            else
                            {
                                execv("./process.out", NULL);
                            }
                            fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), checkedWaitingProcess.memSize, checkedWaitingProcess.idTextFile, toProcesstemp->start, toProcesstemp->end);
                        }
                        //////////////////////////////////////////////////////////////////////////////////////////////
                    }
                        terminatingcondition--;
                        // printf("After Deletion: \n");
                        // printArray(readySRTNQueue, size);
                    }
                    int incProcressId = peekRootId(readySRTNQueue);
                    prevTempProcessID = incProcressId;
                    int incProcessRunningTime = peekRootRunningTime(readySRTNQueue);
                    struct msgbuffToProcess sendMessage;
                    sendMessage.mtype = incProcressId;
                    sendMessage.runningtime = incProcessRunningTime;
                    readySRTNQueue[0].runningtime = readySRTNQueue[0].runningtime - 1;
                    if (incProcessRunningTime != -1)
                    {

                        // printf("clock = %d i am process %d ,running time= %d\n", getClk(), incProcressId, incProcessRunningTime);
                        adjustPCB(&PCB, Processes, incProcressId, prevProcessID, &WTAvalues);
                        int checkSend = msgsnd(upid, &sendMessage, sizeof(sendMessage.runningtime), !IPC_NOWAIT); // tells process to run
                        // Failed to receive message.
                        if (checkSend == -1)
                        {
                            perror("Error in send");
                        }
                        prevProcessID = prevTempProcessID;
                        // previousProcess = readySRTNQueue[0];
                    }
                }
            }
        }
    }

    if (atoi(argv[1]) == 3) // RR
    {
        int countToQuantum = 0;
        while (1)
        {
            if (terminatingcondition == 0)
            {
                simEndTime = getClk();
                schedulerPerf(&WTAvalues);
                break;
            }
            down(sem1);
            struct msgbuff recMessage;

            // Failed to receive message.

            while (msgrcv(msgq_id_s, &recMessage, sizeof(recMessage.sentProcess), 0, IPC_NOWAIT) > 0) // tries to receive a process
            {

                // Check for allocation space
                struct Node *bestFit = NULL;
                searchBestFit(root, recMessage.sentProcess.memSize, &bestFit);
                if (bestFit == NULL)
                {
                    // put in waiting list
                    enqueue(&waitingList, recMessage.sentProcess);
                }
                else
                {
                    struct Node *forProcesstemp = NULL;
                    allocateSpace(bestFit, recMessage.sentProcess.memSize, &forProcesstemp);
                    recMessage.sentProcess.allocationSpace = forProcesstemp;
                    fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), recMessage.sentProcess.memSize, recMessage.sentProcess.idTextFile, forProcesstemp->start, forProcesstemp->end);

                    int newPid = fork();

                    if (newPid != 0)
                    {
                        recMessage.sentProcess.id = newPid;
                        if (atoi(argv[1]) == 3) // RR
                        {
                            PCB[PCBindex] = recMessage.sentProcess;
                            PCBindex++;
                            if (firstprocess == 0)
                            {
                                firstprocess = 1;
                                simStartTime = recMessage.sentProcess.arrivaltime;
                                // printf(" arrival time=%d\n",simStartTime);
                            }
                            enqueue(&readyRRQueue, recMessage.sentProcess); // inserted in readyqueue
                        }
                    }
                    else
                    {
                        execv("./process.out", NULL);
                    }
                }
            }

            {
                struct processData incProcess = peek(&readyRRQueue);
                int incProcessId = incProcess.id;
                prevTempProcessID = incProcessId;
                // printf("process to be peeked= %d",incProcessId);
                if (incProcessId != -1)
                {
                    struct msgbuffToProcess recMessageFromProcess;
                    int rec_valFromProcess = msgrcv(downid, &recMessageFromProcess, sizeof(recMessageFromProcess.runningtime), 0, IPC_NOWAIT);
                    // Failed to receive message.
                    if (rec_valFromProcess == -1)
                    {
                        // perror("Error in receive");
                    }
                    else
                    {
                        int processIdToKill = recMessageFromProcess.mtype;
                        struct processData deletedProcess = dequeueByID(&readyRRQueue, processIdToKill);
                        fprintf(fileobj3, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), deletedProcess.memSize, deletedProcess.idTextFile, deletedProcess.allocationSpace->start, deletedProcess.allocationSpace->end);
                        freeAllocatedResource(&deletedProcess.allocationSpace, root);
                    while(1)
                    {

                        
                        struct processData checkedWaitingProcess = peek(&waitingList);
                        struct Node *checkBestFit = NULL;
                        if (checkedWaitingProcess.id != -1)
                        {
                            searchBestFit(root, checkedWaitingProcess.memSize, &checkBestFit);
                        }

                        if (checkedWaitingProcess.id == -1)
                        {
                           break;
                        }

                         if (checkBestFit == NULL)
                        {
                            break;
                        }

                        if (checkBestFit != NULL)
                        {
                            dequeue(&waitingList);
                            struct Node *toProcesstemp = NULL;
                            allocateSpace(checkBestFit, checkedWaitingProcess.memSize, &toProcesstemp);
                            checkedWaitingProcess.allocationSpace = toProcesstemp;
                            /////////////////////////////////////////////////////////////////////////////////////
                            int newPid = fork();

                            if (newPid != 0)
                            {
                                checkedWaitingProcess.id = newPid;
                                if (atoi(argv[1]) == 3) // RR
                                {
                                    checkedWaitingProcess.waitingTime = getClk() - checkedWaitingProcess.arrivaltime;
                                    PCB[PCBindex] = checkedWaitingProcess;
                                    PCBindex++;
                                    enqueue(&readyRRQueue, checkedWaitingProcess); // inserted in readyqueue
                                }
                            }
                            else
                            {
                                execv("./process.out", NULL);
                            }
                            fprintf(fileobj3, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), checkedWaitingProcess.memSize, checkedWaitingProcess.idTextFile, toProcesstemp->start, toProcesstemp->end);
                        }
                        //////////////////////////////////////////////////////////////////////////////////////////////
                    }
                        terminatingcondition--;
                        countToQuantum = 0;
                        // printf("After Deletion: ");
                        // print(&readyRRQueue);
                    }

                    struct processData incProcess = peek(&readyRRQueue);
                    int incProcessId = incProcess.id;
                    prevTempProcessID = incProcessId;
                    int incProcessRunningTime = incProcess.runningtime;
                    struct msgbuffToProcess sendMessage;
                    sendMessage.mtype = incProcessId;
                    sendMessage.runningtime = incProcessRunningTime;
                    if (incProcessId != -1)
                    {
                        printf("Entering to send at %d for process of id %d\n", getClk(), incProcessId);
                        // printf("clock = %d i am process %d ,running time= %d\n", getClk(), incProcessId, incProcessRunningTime);
                        adjustPCB(&PCB, Processes, incProcessId, prevProcessID, &WTAvalues); // adjust pcb handles scheduler pcb as well as printing to log files
                        countToQuantum++;
                        int checkSend = msgsnd(upid, &sendMessage, sizeof(sendMessage.runningtime), !IPC_NOWAIT); // tells process to run
                        // Failed to receive message.
                        if (checkSend == -1)
                        {
                            perror("Error in send");
                        }
                        // previousProcess = incProcess;
                        prevProcessID = prevTempProcessID;
                    }
                    if (countToQuantum == atoi(argv[2]))
                    {
                        // printf("at time %d I will circulate\n", getClk());
                        enqueue(&readyRRQueue, dequeue(&readyRRQueue)); // circular queue
                        countToQuantum = 0;
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fclose(fileobj);
    fclose(fileobj2);
    fclose(fileobj3);
    destroyClk(true);
}
