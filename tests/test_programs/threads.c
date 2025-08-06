#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

void *thread_func(void *arg) {
    int id = *(int *)arg;
    time_t start = time(NULL);

    while (time(NULL) - start < 60) {
        printf("Thread %d: running...\n", id);
        sleep(5);
    }

    printf("Thread %d: done\n", id);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    if (pthread_create(&t1, NULL, thread_func, &id1) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&t2, NULL, thread_func, &id2) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Main thread: done\n");
    return 0;
}
