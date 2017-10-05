#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <semaphore.h>

void *func(void *);

sem_t sem;

int main() {
    pthread_t tid;
    
    sem_init(&sem,0, 1);
    sem_wait(&sem);

    pthread_create(&tid, NULL, func, NULL);

    // Wait for user input
    getchar();
    
    sem_post(&sem);

    // Wait for thread to finish
    pthread_join(tid, NULL);
    return 0;
}

void *func(void *arg) {
    sem_wait(&sem);

    printf("func called\n");
    
    sem_post(&sem);
}
