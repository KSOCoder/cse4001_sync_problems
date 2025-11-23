//
// Example from: http://www.amparo.net/ce155/sem-ex.c
//
// Adapted using some code from Downey's book on semaphores
//
// Compilation:
//
//       g++ main.cpp -lpthread -o main -lm
// or 
//      make
//

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#include <iostream>
using namespace std;

/*
 This wrapper class for semaphore.h functions is from:
 http://stackoverflow.com/questions/2899604/using-sem-t-in-a-qt-project
 */
class Semaphore {
public:
    // Constructor
    Semaphore(int initialValue)
    {
        sem_init(&mSemaphore, 0, initialValue);
    }
    // Destructor
    ~Semaphore()
    {
        sem_destroy(&mSemaphore); /* destroy semaphore */
    }
    
    // wait
    void wait()
    {
        sem_wait(&mSemaphore);
    }
    // signal
    void signal()
    {
        sem_post(&mSemaphore);
    }
    
    
private:
    sem_t mSemaphore;
};


// Global variables
const int numReaders = 5;
const int numWriters = 5; 
const int numPhilosophers = 5; 

// Problem 1: No-starve Readers-Writers
Semaphore roomEmpty1(1);
Semaphore turnstile1(1);
int readers1 = 0;
Semaphore mutex1(1);

void *NoStarveReader(void *threadID)
{
   int id = *((int*)threadID);
   while (1) {
        sleep(2);

        turnstile1.wait();
        turnstile1.signal();

        mutex1.wait();
        readers1++;
        if (readers1 == 1) {
            roomEmpty1.wait();
        }
        mutex1.signal();

        printf("Reader %d is reading...\n", id);
        fflush(stdout);
        sleep(1);

        mutex1.wait();
        readers1--;
        if (readers1 == 0) {
            roomEmpty1.signal();
        }
        mutex1.signal();
   }
   return NULL;
}

void *NoStarveWriter(void *threadID)
{
   int id = *((int*)threadID);
   sleep(id);
   while (1) {
        sleep(6);

        turnstile1.wait();
        roomEmpty1.wait();

        printf("Writer %d is writing...\n", id);
        fflush(stdout);
        sleep(2);

        turnstile1.signal();
        roomEmpty1.signal();
   }
   return NULL;
}

// Problem 2: Writer-priority Readers-Writers
int readers2 = 0;
int writers2 = 0;
Semaphore readMutex2(1);
Semaphore writeMutex2(1);
Semaphore readTry2(1);
Semaphore resource2(1);

void *WriterPriorityReader(void *threadID)
{
   int id = *((int*)threadID);
   while (1) {
        sleep(2);
        readTry2.wait();
        readMutex2.wait();
        readers2++;
        if (readers2 == 1) {
            resource2.wait();
        }
        readMutex2.signal();
        readTry2.signal();

        printf("Reader %d is reading...\n", id);
        fflush(stdout);
        sleep(1);

        readMutex2.wait();
        readers2--;
        if (readers2 == 0) {
            resource2.signal();
        }
        readMutex2.signal();
   }
   return NULL;
}

void *WriterPriorityWriter(void *threadID)
{
   int id = *((int*)threadID);
   sleep(id * 2);
   while (1) {
        sleep(12);

        writeMutex2.wait();
        writers2++;
        if (writers2 == 1) {
            readTry2.wait();
        }
        writeMutex2.signal();

        resource2.wait();

        printf("Writer %d is writing...\n", id);
        fflush(stdout);
        sleep(2);

        resource2.signal();

        writeMutex2.wait();
        writers2--;
        if (writers2 == 0) {
            readTry2.signal();
        }
        writeMutex2.signal();
   }
   return NULL;
}


// Problem 3: Dining Philosophers #1
enum {THINKING, HUNGRY, EATING};
int state3[numPhilosophers];
Semaphore philMutex3(1);
Semaphore s3[numPhilosophers] = {Semaphore(0), Semaphore(0), Semaphore(0), Semaphore(0), Semaphore(0)};

void test(int i) {
    if (state3[i] == HUNGRY && state3[(i + numPhilosophers - 1) % numPhilosophers] != EATING && state3[(i + 1) % numPhilosophers] != EATING) {
        state3[i] = EATING;
        s3[i].signal();
    }
}

void takeForks3(int i) {
    philMutex3.wait();
    state3[i] = HUNGRY;
    printf("Philosopher %d is HUNGRY...\n", i);
    fflush(stdout);
    test(i);
    philMutex3.signal();
    s3[i].wait();
}

void putForks3(int i) {
    philMutex3.wait();
    state3[i] = THINKING;
    test((i + numPhilosophers - 1) % numPhilosophers);
    test((i + 1) % numPhilosophers);
    philMutex3.signal();
}

void *Philosopher3(void *threadID)
{
   int id = *((int*)threadID);
   while (1) {
        printf("Philosopher %d is THINKING...\n", id);
        fflush(stdout);
        sleep(rand() % 3 + 5);

        takeForks3(id);

        printf("Philosopher %d is EATING...\n", id);
        fflush(stdout);
        sleep(2);

        putForks3(id);
   }
   return NULL;
}

// Problem 4: Dining Philosophers #2
Semaphore fork4[numPhilosophers] = {Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1)};

void *Philosopher4(void *threadID)
{
   int id = *((int*)threadID);
   int left = id - 1;
   int right = id % numPhilosophers;

   while (1) {
        printf("Philosopher %d is THINKING...\n", id);
        fflush(stdout);
        sleep(rand() % 3 + 5);

        printf("Philosopher %d is HUNGRY...\n", id);
        fflush(stdout);

        if (id % 2 == 0) {
            fork4[left].wait();
            fork4[right].wait();
        } else {
            fork4[right].wait();
            fork4[left].wait();
        }

        printf("Philosopher %d is EATING...\n", id);
        fflush(stdout);
        sleep(2);

        fork4[left].signal();
        fork4[right].signal();
   }
   return NULL;
}

int main(int argc, char **argv )
{
    if (argc != 2) {
        printf("Usage: %s <problem_number>\n", argv[0]);
        printf("Problems:\n");
        printf("  1: No-starve Readers-Writers\n");
        printf("  2: Writer-priority Readers-Writers\n");
        printf("  3: Dining Philosophers #1\n");
        printf("  4: Dining Philosophers #2\n");
        return 1;
    }

    int problem = atoi(argv[1]);
    pthread_t threads[10];
    int threadIDs[10];

    srand(time(NULL));

    switch(problem){
        case 1:
            printf("Running No-starve Readers-Writers solution\n");
            printf("==========================================\n\n");
            for (long i = 0; i < numReaders; i++) {
                threadIDs[i] = i + 1;
                pthread_create(&threads[i], NULL, NoStarveReader, &threadIDs[i]);
            }
            for (long i = 0; i < numWriters; i++) {
                threadIDs[numReaders + i] = i + 1;  
                pthread_create(&threads[numReaders + i], NULL, NoStarveWriter, &threadIDs[numReaders + i]);
            }
            break;

        case 2:
            printf("Running Writer-priority Readers-Writers solution\n");
            printf("================================================\n\n");
            for (long i = 0; i < numReaders; i++) {
                threadIDs[i] = i + 1;
                pthread_create(&threads[i], NULL, WriterPriorityReader, &threadIDs[i]);
            }
            for (long i = 0; i < numWriters; i++) {
                threadIDs[numReaders + i] = i + 1;
                pthread_create(&threads[numReaders + i], NULL, WriterPriorityWriter, &threadIDs[numReaders + i]);
            }
            break;

        case 3:
            printf("Running Dining Philosophers #1 solution\n");
            printf("=======================================\n\n");
            for (long i = 0; i < numPhilosophers; i++) {
                state3[i] = THINKING;
                threadIDs[i] = i + 1;
                pthread_create(&threads[i], NULL, Philosopher3, &threadIDs[i]);
            }
            break;

        case 4:
            printf("Running Dining Philosophers #2 solution\n");
            printf("=======================================\n\n");
            for (long i = 0; i < numPhilosophers; i++) {
                threadIDs[i] = i + 1;
                pthread_create(&threads[i], NULL, Philosopher4, &threadIDs[i]);
            }
            break;

        default:
            printf("Invalid problem number. Choose 1-4.\n");
            return 1;
    }

    printf("Main: program completed. Exiting.\n");

    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 
    return 0;
}