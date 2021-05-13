#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include "a2_helper.h"

#define P7TH 5
#define P3TH 6
#define P5TH 39

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int counter = 0;
int semId;

// Decrement the semaphore value - AKA LOCK
void P(int semId, int sem_no)
{
    struct sembuf semaphoreOperation = {0, -1, 0};
    semop(semId, &semaphoreOperation, 1);
}

// Increment the value - AKA UNLOCK
void V(int semId, int sem_no)
{
    struct sembuf semaphoreOperation = {0, 1, 0};
    semop(semId, &semaphoreOperation, 1);
}

void* p7Threads(void* arg) 
{
    int id = *((int*) arg);
    
    if(id == 2)
    {
        P(semId, 0);
    }

    info(BEGIN, 7, id);
    if(id == 3)
    {
        V(semId, 0);
    }

    if(id == 3)
    {
        P(semId, 1);
    }

    info(END, 7, id);
    if(id == 2)
    {
        V(semId, 1);
    }
    return NULL;
}
void* p3Threads(void* arg) 
{
    int id = *((int*) arg); 
        
    info(BEGIN, 3, id);
    info(END, 3, id);
   
    return NULL;
}
void* p5Threads(void* arg) 
{
    int id = *((int*) arg);
    
    pthread_mutex_lock(&lock);
    while(counter == 4)
    {
        pthread_cond_wait(&cond, &lock);
    }
    counter++;
    pthread_mutex_unlock(&lock);

    info(BEGIN, 5, id);
    
    info(END, 5, id);
    
    pthread_mutex_lock(&lock);
    counter--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    
    return NULL;
}

int p7threadsMain()
{
    pthread_t threads[P7TH];
    int threadIndex[P7TH];

    info(BEGIN, 7, 0);
    semId = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if(semId < 0)
    {
        perror("Cannot create sem");
        exit(2);
    }

    semctl(semId, 0, SETVAL, 0);
    semctl(semId, 1, SETVAL, 0);

    for(int i = 1; i <= P7TH; i++)
    {
        threadIndex[i] = i;
        pthread_create(&threads[i], NULL, p7Threads, &threadIndex[i]);
    }

    for(int i = 1; i <= P7TH; i++)
    {
        pthread_join(threads[i], NULL);
    }
    info(END, 7, 0);
    return 0;
}

int p3threadsMain()
{
    pthread_t threads[P3TH];
    int threadIndex[P3TH];

    info(BEGIN, 3, 0);
    for(int i = 1; i <= P3TH; i++)
    {
        threadIndex[i] = i;
        pthread_create(&threads[i], NULL, p3Threads, &threadIndex[i]);
    }

    for(int i = 1; i <= P3TH; i++)
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

int p5threadsMain()
{
    pthread_t threads[P5TH];
    int threadIndex[P5TH];

    for(int i = 1; i <= P5TH; i++)
    {
        threadIndex[i] = i;
        pthread_create(&threads[i], NULL, p5Threads, &threadIndex[i]);
    }

    for(int i = 1; i <= P5TH; i++)
    {
        pthread_join(threads[i], NULL);
    }
    info(END, 5, 0);
    return 0;
}

int main()
{
    init();

    pid_t pid = fork();
    switch (pid) 
    {//P1 is the main process in the hierarchy
        case -1:
        // error case
        {   perror("Cannot create a new child");
            exit(1);
        }
        case 0:
        {//P2 is a child of P1 with different pid
            pid_t pid1 = fork();
            switch (pid1) 
            {
                case -1:
                // error case
                {   perror("Cannot create a new child");
                    exit(1);
                }
                case 0:
                {//P3 is a child of P2 with different pid
                    pid_t pid2 = fork();
                    switch (pid2) 
                    {
                        case -1:
                        // error case
                        {   perror("Cannot create a new child");
                            exit(1);
                        }
                        case 0:
                        {//P6 is a child of P3 with different pid
                            pid_t pid7 = fork();
                            switch (pid7) 
                            {
                                case -1:
                                // error case
                                {   perror("Cannot create a new child");
                                    exit(1);
                                }
                                case 0:
                                { 
                                    pid_t pid8 = fork();
                                    switch (pid8) 
                                    {//P9 is a child of P6 with different pid
                                        case -1:
                                        // error case
                                        {   perror("Cannot create a new child");
                                            exit(1);
                                        }
                                        case 0:
                                        {
                                        }
                                        break;
                                        default:
                                        {
                                            info(BEGIN, 9, 0);
                                            info(END, 9, 0);
                                        } 
                                    }  
                                }
                                break;
                                default:
                                {
                                    info(BEGIN, 6, 0);
                                    waitpid(pid7, NULL, 0);
                                    info(END, 6, 0);
                                } 
                            }
                        }
                        break;
                        default:
                        {
                            p3threadsMain();
                            waitpid(pid2, NULL, 0);
                            info(END, 3, 0);
                        } 
                    }
                }
                break;
                default:
                {
                    info(BEGIN, 2, 0);
                    waitpid(pid1, NULL, 0);
                    //P4 is a child of P2 with different pid 
                    pid_t pid3 = fork();
                    switch (pid3) 
                    {
                        case -1:
                        // error case
                        {   perror("Cannot create a new child");
                            exit(1);
                        }
                        case 0:
                        { 
                            info(BEGIN, 4, 0);
                            info(END, 4, 0);
                        }
                        break;
                        default:
                        {
                            waitpid(pid3, NULL, 0); 
                            info(END, 2, 0);
                        } 
                    }
                }
            }
        }
        break;
        default:
        {
            info(BEGIN, 1, 0);
            waitpid(pid, NULL, 0);
            //P5 is a child of P1 with different pid 
            pid_t pid4 = fork();
            switch (pid4) 
            {
                case -1:
                // error case
                {   perror("Cannot create a new child");
                    exit(1);
                }
                case 0:
                // child
                { 
                    info(BEGIN, 5, 0);
                    //P8 is a child of P5 with different pid
                    pid_t pid6 = fork();
                    switch (pid6) 
                    {
                        case -1:
                        // error case
                        {   perror("Cannot create a new child");
                            exit(1);
                        }
                        case 0:
                        { // child
                            info(BEGIN, 8, 0);
                            info(END, 8, 0);
                        }
                        break;
                        default:
                        {
                            waitpid(pid6, NULL, 0);
                        } 
                       p5threadsMain();
                    }
                }
                break;
                default:
                {
                    //P7 is a child of P1 with different pid 
                    pid_t pid5 = fork();
                    switch (pid5) 
                    {
                        case -1:
                        // error case
                        {   perror("Cannot create a new child");
                            exit(1);
                        }
                        case 0:
                        {
                            p7threadsMain();
                        }
                        break;
                        default:
                        {
                            waitpid(pid4, NULL, 0);
                            waitpid(pid5, NULL, 0);
                        }
                        info(END, 1, 0);
                    }
                } 
            }
        }
    }
    return 0;
}