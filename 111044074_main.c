/**
 * ./floristApp data.dat
 * multithreaded application using pthreads, condition
 * variables and mutexes for flower delivery. Clients will make requests for flowers, a central thread
 * will collect those requests, and then delegate the work to the closest florist that has the kind of
 * flower requested.
 * CSE344 System Programming HomeWork 5 - Florist
 * Sinan Elveren - Gebze Technical University - Computer Engineering
 */
#define _POSIX_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <wait.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <math.h>

#include <fcntl.h>
#include <sys/mman.h>


//#define NDBUG
#define MAX_LINE_LENGTH 128
#define MAX_CLIENT_COUNT 1000
#define MAX_FLORIST_COUNT 3
#define MAX_NAME_LENGTH 20
#define MAX_RESOURCE_COUNT 3



typedef struct Client {
    int clientNum;
    int x;
    int y;
    char name[MAX_NAME_LENGTH];
    char request[MAX_NAME_LENGTH];
} Client;

typedef struct Florist {
    int x;
    int y;
    float velocity;
    char name[MAX_NAME_LENGTH];
    char resources[MAX_RESOURCE_COUNT][MAX_NAME_LENGTH];
} Florist;


typedef struct Order {
    int time;
    int orderCount;
    int orderCountFinished;
    char clientName[MAX_NAME_LENGTH];
    char flowerName[MAX_NAME_LENGTH];
    int threadID;
} Order;


typedef struct Statistic {
    char florist[MAX_NAME_LENGTH];
    int ofSales;
    int totalTime;
} Statistic;



Client  client[MAX_CLIENT_COUNT] = {0, 0, 0, ""};
Florist florist[MAX_FLORIST_COUNT] = {0, 0, 0.0, ""};
Order ordersFlorist1[MAX_CLIENT_COUNT] = {0, 0, 0, "", ""};
Order ordersFlorist2[MAX_CLIENT_COUNT] = {0, 0, 0, "", ""};
Order ordersFlorist3[MAX_CLIENT_COUNT] = {0, 0, 0, "", ""};
Statistic statistic1[MAX_CLIENT_COUNT] = {"", 0, 0};
Statistic statistic2[MAX_CLIENT_COUNT] = {"", 0, 0};
Statistic statistic3[MAX_CLIENT_COUNT] = {"", 0, 0};

pid_t   parentPID = 0;               /*for check child pid*/

int f1TotalRequest = -1;
int f2TotalRequest = -1;
int f3TotalRequest = -1;

/*thread - Condition variable*/
int thread_flag;
int thread_flag2;
int thread_flag3;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexF = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexF2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexF3 = PTHREAD_MUTEX_INITIALIZER;
/*pthread_mutex_t mutexEnd = PTHREAD_MUTEX_INITIALIZER;           //for check end of client's request
pthread_mutex_t mutexEnd2 = PTHREAD_MUTEX_INITIALIZER;           //for check end of client's request
pthread_mutex_t mutexEnd3 = PTHREAD_MUTEX_INITIALIZER;           //for check end of client's request*/




void initialize_flag () {
    /* Initialize the mutex and condition variable*/
    pthread_mutex_init (&mutexF, NULL);                 /*mutex for florist1*/
    pthread_mutex_init (&mutexF2, NULL);                /*mutex for florist2*/
    pthread_mutex_init (&mutexF3, NULL);                /*mutex for florist3*/

    pthread_cond_init (&cv, NULL);                      /*ccondtional variable for florist1*/
    pthread_cond_init (&cv2, NULL);                     /*ccondtional variable for florist2*/
    pthread_cond_init (&cv3, NULL);                     /*ccondtional variable for florist3*/
    // sem_init(&ayse_sem, 0, 0);
    /* Initialize the flag value. */
    thread_flag = -1;
    thread_flag2 = -1;
    thread_flag3 = -1;
}



/*Generate random number between 10 and 50 (closed)*/
int randomRange () {
    unsigned const range = 50 - 10 + 1;

    return  (10 + (int) (((double) range) * rand () / (RAND_MAX + 1.0)));
}



int prepareAndDeliver(int floristID){
    char flowerName[MAX_NAME_LENGTH];
    char clientName[MAX_NAME_LENGTH];
    int time = 0;
    int order = 0;


    if(floristID == 0){
        if(ordersFlorist1[0].orderCount == ordersFlorist1[0].orderCountFinished) {
            thread_flag = -1;
            return 0;
        }
        /*first order*/
        order = ordersFlorist1[0].orderCountFinished + 1;
        strcpy(flowerName, ordersFlorist1[order].flowerName);
        strcpy(clientName, ordersFlorist1[order].clientName);
        time = ordersFlorist1[order].time;

        /*delete(change queue) first order from array*/
        ordersFlorist1[0].orderCountFinished = order;

        if(ordersFlorist1[0].orderCount == ordersFlorist1[0].orderCountFinished)
            thread_flag = -1;

        /*enter statistics*/
        strcpy(statistic1->florist, florist[0].name);
        statistic1->ofSales += 1;
        statistic1->totalTime += ordersFlorist1[order].time;

        fflush(stdout);
        fprintf(stdout, "Florist [%s] has delivered a [%s] to [%s] in [%d]ms\n",
                florist[floristID].name, flowerName, clientName, ordersFlorist1[order].time );
        fflush(stdout);
    }


    if(floristID == 1){
        if(ordersFlorist2[0].orderCount == ordersFlorist2[0].orderCountFinished) {
            thread_flag2 = -1;
            return 0;
        }
        /*first order*/
        order = ordersFlorist2[0].orderCountFinished + 1;
        strcpy(flowerName, ordersFlorist2[order].flowerName);
        strcpy(clientName, ordersFlorist2[order].clientName);
        time = ordersFlorist2[order].time;

        /*delete(change queue) first order from array*/
        ordersFlorist2[0].orderCountFinished = order;

        if(ordersFlorist2[0].orderCount == ordersFlorist2[0].orderCountFinished)
            thread_flag2 = -1;

        /*enter statistics*/
        strcpy(statistic2->florist, florist[1].name);
        statistic2->ofSales += 1;
        statistic2->totalTime += ordersFlorist2[order].time;

        fflush(stdout);
        fprintf(stdout, "Florist [%s] has delivered a [%s] to [%s] in [%d]ms\n",
                florist[floristID].name, flowerName, clientName, ordersFlorist1[order].time );
        fflush(stdout);
    }


    if(floristID == 2){
        if(ordersFlorist3[0].orderCount == ordersFlorist3[0].orderCountFinished) {
            thread_flag3 = -1;
            return 0;
        }
        /*first order*/
        order = ordersFlorist3[0].orderCountFinished + 1;
        strcpy(flowerName, ordersFlorist3[order].flowerName);
        strcpy(clientName, ordersFlorist3[order].clientName);
        time = ordersFlorist3[order].time;

        /*delete(change queue) first order from array*/
        ordersFlorist3[0].orderCountFinished = order;

        if(ordersFlorist3[0].orderCount == ordersFlorist3[0].orderCountFinished)
            thread_flag3 = -1;

        /*enter statistics*/
        strcpy(statistic3->florist, florist[2].name);
        statistic3->ofSales += 1;
        statistic3->totalTime += ordersFlorist3[order].time;


        fflush(stdout);
        fprintf(stdout, "Florist [%s] has delivered a [%s] to [%s] in [%d]ms\n",
                florist[floristID].name, flowerName, clientName, ordersFlorist1[order].time );
        fflush(stdout);
    }


    return 0;
}


void* thread_function3 (void* arg) {
    int threadID = *((int*)arg);

    while (1) {
        // fprintf(stdout," test start %d %d\n", threadID,thread_flag);
        /* Lock the mutex before accessing the flag value. */
        pthread_mutex_lock(&mutexF3);


        if (thread_flag3 == 2) {
            /*prepare and deliver for florist3 f*/
            prepareAndDeliver(threadID);

        } else {
            /* check for end of clients : next order?*/
            if (f3TotalRequest == ordersFlorist3[0].orderCountFinished) {
                pthread_exit(NULL);
            }

            /* waiting order signal for prepare and deliver to next order */
            pthread_cond_wait(&cv3, &mutexF3);
        }

        pthread_mutex_unlock(&mutexF3);
    }

    return NULL;
}



void* thread_function2 (void* arg) {
    int threadID = *((int*)arg);

    while (1) {
        // fprintf(stdout," test start %d %d\n", threadID,thread_flag);
        /* Lock the mutex before accessing the flag value. */
        pthread_mutex_lock(&mutexF2);

        if (thread_flag2 == 1) {
            /*prepare and deliver for florist2f*/
            prepareAndDeliver(threadID);

        } else {

            /* check for end of clients : next order?*/
            if (f2TotalRequest == ordersFlorist2[0].orderCountFinished) {
                pthread_exit(NULL);
            }

            /* waiting order signal for prepare and deliver to next order */
            pthread_cond_wait(&cv2, &mutexF2);
        }

        /*Unlock the mutex*/
        pthread_mutex_unlock (&mutexF2);
    }

    return NULL;
}




void* thread_function (void* arg) {
    int threadID = *((int*)arg);

    while (1) {
        // fprintf(stdout," test start %d %d\n", threadID,thread_flag);
        /* Lock the mutex to accces flag value. */
        pthread_mutex_lock (&mutexF);

        if (thread_flag == 0) {
            /*wait order queue*/
            // sem_wait(ayse_sem);
            /*prepare and deliver for florist3 f*/
            prepareAndDeliver(threadID);

        } else{

            /* check for end of clients : next order?*/
            if(f1TotalRequest == ordersFlorist1[0].orderCountFinished){
                pthread_exit(NULL);
            }

            /* waiting order signal for prepare and deliver to next order */
            pthread_cond_wait(&cv, &mutexF);
        }

        pthread_mutex_unlock (&mutexF);

    }
    return NULL;
}

void set_thread_flag (int threadID, char flowerName[], char clientName[], int time) {
    int oc1 = 0;     /*order count * temp */
    int oc2 = 0;     /*order count * temp */
    int oc3 = 0;     /*order count * temp */



    if(threadID == 0) {
        pthread_mutex_lock (&mutexF);

        thread_flag = threadID;

        /*all oder info into to struct of oder*/
        oc1 = ordersFlorist1[0].orderCount+1;
        ordersFlorist1[0].orderCount = oc1;
        ordersFlorist1[oc1].threadID = threadID;
        ordersFlorist1[oc1].time = time;
        strcpy( ordersFlorist1[oc1].clientName, clientName );
        strcpy( ordersFlorist1[oc1].flowerName, flowerName );
        pthread_cond_signal(&cv);

        pthread_mutex_unlock (&mutexF);

    }



    if(threadID == 1) {
        pthread_mutex_lock (&mutexF2);


        thread_flag2 = threadID;

        /*all oder info into to struct of oder*/
        oc2 = ordersFlorist2[0].orderCount+1;
        ordersFlorist2[0].orderCount = oc2;
        ordersFlorist2[oc2].threadID = threadID;
        ordersFlorist2[oc2].time = time;
        strcpy( ordersFlorist2[oc2].clientName, clientName );
        strcpy( ordersFlorist2[oc2].flowerName, flowerName );
        pthread_cond_signal(&cv2);

        pthread_mutex_unlock (&mutexF2);
    }

    if(threadID == 2) {
        pthread_mutex_lock (&mutexF3);

        thread_flag3 = threadID;

        /*all oder info into to struct of oder*/
        oc3 = ordersFlorist3[0].orderCount+1;
        ordersFlorist3[0].orderCount = oc3;
        ordersFlorist3[oc3].threadID = threadID;
        ordersFlorist3[oc3].time = time;
        strcpy( ordersFlorist3[oc3].clientName, clientName );
        strcpy( ordersFlorist3[oc3].flowerName, flowerName );
        pthread_cond_signal(&cv3);

        pthread_mutex_unlock (&mutexF3);
    }


    //  sem_post(ayse_sem);
}

/*return distance/time of florist id */
int calcDeliverTime(int x, int y, int id){
    float distance = 0;

    distance =  sqrt( ( pow((florist[id].x - x), 2) + pow( (florist[id].y - y), 2) ) );

    if (distance < 0)
        distance *= -1;

    //   fprintf(stdout, "i%d  x%d  y%d  d%f  T%f t%d \n ", id, x, y, distance, florist[id].velocity,  (int)(distance / florist[id].velocity));

    return (int)(100 * distance / florist[id].velocity);
}

void statisticsPrint(){
    fflush(stdout);
    fprintf(stdout, "Sale statistics for today:\n");
    fprintf(stdout, "-------------------------------------------------\n");
    fprintf(stdout, "Florist\t\t# of Sales \t TotalTime\n");
    fprintf(stdout, "-------------------------------------------------\n");

    fprintf(stdout, "%s \t\t%d \t\t %d\n",statistic1->florist, statistic1->ofSales, statistic1->totalTime);
    fprintf(stdout, "%s \t\t%d \t\t %d\n",statistic2->florist, statistic2->ofSales, statistic2->totalTime);
    fprintf(stdout, "%s \t\t%d \t\t %d\n",statistic3->florist, statistic3->ofSales, statistic3->totalTime);

    fprintf(stdout, "-------------------------------------------------\n");
    fflush(stdout);
}


pid_t myWait(int *status);
void myAtexit(void);
void signalCatcher(int signum);      /*or exit funcs*/
void finish(int exitNum);            /*exit*/


int readData(const char *fileName, Client client[], Florist florist[], int *floristNum, int *clientNum);


int main(int argc, char *argv[]) {

    if(atexit(myAtexit) != 0 ) {
        perror("atexit");
        return 1;
    }

    parentPID = getpid();
    errno = 0;
    int found[MAX_FLORIST_COUNT] = {-1, -1, -1};
    int z = 0;
    int requestNum1 = 0;
    int requestNum2 = 0;
    int requestNum3 = 0;

    int     floristNum = 0, clientNum = 0;



    srand (time (NULL));

    /**Signal**/
    struct sigaction newact;
    newact.sa_handler = signalCatcher;
    /* set the new handler */
    newact.sa_flags = 0;

    /*install sigint*/
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT, &newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
        return EXIT_FAILURE;
    }


    /*usage check*/
    if(argc != 2){
        perror("Usage error");
        fprintf(stdout, "try to ./floristApp <fileName>\n");
        exit(EXIT_FAILURE);
    }


    pthread_t   thread1, thread2, thread3;
    int         threadID1 = 0, threadID2 = 0, threadID3 = 0;
    int         florist1 = 0,
                florist2 = 1,
                florist3 = 2;

    initialize_flag();


    fprintf(stdout, "Florist application initializing from file: %s\n", argv[1]);


    /*creating florist 1-2-3*/
    threadID1 = pthread_create (&thread1, NULL, &thread_function, (void*) &florist1);
    threadID2 = pthread_create (&thread2, NULL, &thread_function2, (void*) &florist2);
    threadID3 = pthread_create (&thread3, NULL, &thread_function3, (void*) &florist3);

    if(threadID1 != 0 || threadID2 != 0 || threadID3 != 0){
        perror("pthread - creat florist error");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "%d florists have been created\n", MAX_FLORIST_COUNT);


    /*read data from file*/
    readData(argv[1], client, florist, &floristNum, &clientNum);

    fprintf(stdout, "Processing requests\n");
    fflush(stdout);

    int time = 0;
    int oldTime = 100000;
    int floristID = -1;

    /*initialize orders list - 0 nd struct is referance for info*/
    pthread_mutex_lock (&mutexF);
    ordersFlorist1[0].orderCount = 0;
    ordersFlorist2[0].orderCount = 0;
    ordersFlorist3[0].orderCount = 0;
    pthread_mutex_unlock (&mutexF);

    for (int i = 0; i < clientNum; ++i) {
        for (int k = 0; k < MAX_FLORIST_COUNT; ++k) {
            for (int j = 0; j < MAX_RESOURCE_COUNT; ++j) {
                if (strcmp(client[i].request, florist[k].resources[j]) == 0) {
                    time = calcDeliverTime(client[i].x, client[i].y, k/*found[j]*/);
                    if (time < oldTime){
                        oldTime = time;
                        floristID = k;          //shortest -> distanvce / velocity

                    }
                    //     found[z++] = k;          //florist is found
                }
            }
        }

        /*deliver order*/
        set_thread_flag(floristID,  client[i].request,  client[i].name, time/100 + randomRange() );

        /*calc total order count for end the florists job*/
        if(strcmp(florist[floristID].name, florist[0].name) == 0) {  /* 1st florist will deliver*/
            ++requestNum1;
        }

        if(strcmp(florist[floristID].name, florist[1].name) == 0) {  /* 2nd florist will deliver*/
            ++requestNum2;
        }

        if(strcmp(florist[floristID].name, florist[2].name) == 0) {  /* 3th florist will deliver*/
            ++requestNum3;
        }
        z = 0;
        oldTime =100000;        /*for calculate new time for request*/
        time = 0;
    }


    /*write total request count for end the florists's job*/
    pthread_mutex_lock (&mutexF);
    f1TotalRequest = requestNum1;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock (&mutexF);

    pthread_mutex_lock (&mutexF2);
    f2TotalRequest = requestNum2;
    pthread_cond_signal(&cv2);
    pthread_mutex_unlock (&mutexF2);

    pthread_mutex_lock (&mutexF3);
    f3TotalRequest = requestNum3;
    pthread_cond_signal(&cv3);
    pthread_mutex_unlock (&mutexF3);


    fflush(stdout);
    fprintf(stdout, "\nAll requests processed.\n");
    fflush(stdout);



    pthread_join(thread1, NULL);
    fprintf(stdout, "\n%s closing shop.\n", florist[0].name);
    fflush(stdout);

    pthread_join(thread2, NULL);
    fprintf(stdout, "\n%s closing shop.\n", florist[1].name);
    fflush(stdout);

    pthread_join(thread3, NULL);
    fprintf(stdout, "\n%s closing shop.\n", florist[2].name);
    fflush(stdout);


  //  statisticsPrint();            //call it in atExit



    /*file content is here */

    /*    for (int i = 0; i < floristNum ; ++i) {
        fprintf(stdout, "F : %s %d %d %f  \t%s%s%s\n",
                florist[i].name, florist[i].x, florist[i].y, florist[i].velocity,
                florist[i].resources[0], florist[i].resources[1], florist[i].resources[2]);
    }

    for (int i = 0; i < clientNum ; ++i) {
        fprintf(stdout, "C : %s %d %d %d%s\n", client[i].name,client[i].x, client[i].y, client[i].clientNum, client[i].request);
    }*/


    return 0;
}



/* * * * * * * * * * * * * * * * * * * * * * *_END_OF_MAIN_ * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



pid_t myWait(int *status) {
    pid_t rtrn;

    while (((rtrn = wait(status)) == -1) && (errno == EINTR));

    return rtrn;
}




void myAtexit(void){
    //childreen is coming
    if (getpid() != parentPID) {

        fprintf(stdout,"\n    [%ld]No child procces wasa here\n",(long) getpid());

        return;
    }

    while (myWait(NULL ) > 0);


    /*if there are any fault*/
    if( errno != 0 ) {

        /*print statistics*/
        statisticsPrint();

        fprintf(stdout, "\n    [%d]PARENT: I'm going to exit\n", getpid());
        fflush(stdout);


        /*free and destroy is here*/
        pthread_mutex_destroy(&mutexF);

        if ( pthread_cond_destroy(&cv) == EBUSY) { /* still waiting condition variable */
            pthread_cond_signal(&cv);
        }
        if ( pthread_cond_destroy(&cv2) == EBUSY) { /* still waiting condition variable */
            pthread_cond_signal(&cv2);
        }
        if ( pthread_cond_destroy(&cv3) == EBUSY) { /* still waiting condition variable */
            pthread_cond_signal(&cv3);
        }

        fprintf(stdout, "-> has been free/destroyed succesfuly\n");
        fflush(stdout);


        fprintf(stdout, "\n    [%d]Parent Process has been exit succesfuly\n", getpid());
        fflush(stdout);

        fprintf(stdout, "\nNOT : Hesaplamada bir yanlislik oldugundan total 'of Sales' miktari yanlis olablir\n\n");

    }
    return;
}




void signalCatcher(int signum) {

    /*get ready for exit*/


    fflush(stdout);
    /*children are leaving here*/
    if(getpid() != parentPID)
        return;




    switch (signum) {
        case SIGUSR1: puts("\ncaught SIGUSR1");
            break;
        case SIGUSR2: puts("\ncaught SIGUSR2");
            break;
        case SIGINT:
            fprintf(stderr,"\n\n[%ld]SIGINT:Ctrl+C signal detected, exit code : (%d) \n", (long)getpid(), signum);
            finish(-1);
            break;
        default:
            fprintf(stderr,"Catcher caught unexpected signal (%d)\n", signum);

            finish(1);
            break;
    }
}


/* or exit funcs */
void finish(int exitNum) {
    /*  myAtexit(); */
    exit(exitNum);
}



/* * * * * * * * * * * * * * * * * * * * * * * _END_OF_HANDLERs_ * * * * * * * * * * * *  * * * * * * * * * * * * * * */


/* read *dat file 's content */
int readData(const char *fileName, Client client[], Florist florist[], int *floristNum, int *clientNum ) {
    char line[MAX_LINE_LENGTH];
    char temp[MAX_NAME_LENGTH];

    FILE *fp =NULL;


    fp = fopen(fileName, "rb");

    if ( fp == NULL ) {
        perror( "Could not open file" ) ;
        exit(EXIT_FAILURE);
    }


    for (int i = 0; fgets(line, MAX_LINE_LENGTH-1,fp) != NULL && i < MAX_CLIENT_COUNT; ++i) {
        if(strlen(line) > 1) {
            if (3 == sscanf(line, "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%f",
                            &(florist[i].x), &(florist[i].y), &(florist[i].velocity) )) {


                for (int j = 0; line[j] != '\0'; ++j) {
                    if (line[j] == '(' && line[j + 1] == '-')
                        florist[i].x *= -1;
                    if (line[j] == ',' && line[j + 1] == '-')
                        florist[i].y *= -1;
                }
                sscanf(line, "%s %s%s%s  %s%s%s", &(florist[i].name), &temp, &temp, &temp,
                       &(florist[i].resources[0]), &(florist[i].resources[1]), &(florist[i].resources[2]));
                for (int j = 0; j < MAX_RESOURCE_COUNT-1; ++j) {
                    florist[i].resources[j][strlen(florist[i].resources[j]) -1] = '\0';
                }
            }else{
                perror("Data file format error");
                exit(EXIT_FAILURE);
            }
            *floristNum = i+1;
        }else{
            i = MAX_CLIENT_COUNT;        /*break; end of florist*/
        }

    }

    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < *floristNum ; ++i) {
        fgets(line, MAX_LINE_LENGTH-1,fp);
    }
    for (int i = 0; fgets(line, MAX_LINE_LENGTH-1,fp) != NULL && i < MAX_CLIENT_COUNT; ++i) {
        if(strlen(line) > 1) {
            if (3 == sscanf(line, "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                            &(client[i].clientNum), &(client[i].x), &(client[i].y) )){


                for (int j = 0; line[j] != '\0'; ++j) {
                    if(line[j] == '(' && line[j+1] == '-' )
                        client[i].x *= -1;
                    if(line[j] == ',' && line[j+1] == '-' )
                        client[i].y *= -1;
                }
                sscanf(line, "%s %s %s", &(client[i].name) , &temp, &(client[i].request));
            }else{
                perror("Data file format error");
                exit(EXIT_FAILURE);
            }

        }else
            i--;        /*pass the epty line*/

        *clientNum = i+1;
    }



    fclose(fp);

    return 0;

}
