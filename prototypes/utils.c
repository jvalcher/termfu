#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "data.h"
#include "utils.h"


sem_t *create_semaphore (void)
{
    sem_t *sem = sem_open (DEBUG_SEMAPHORE, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}


void wait_lock_debug_process (sem_t *lock)
{
    if (sem_open(lock) == -1) {
        fprintf (stderr, "Debug semaphore lock");
        exit (EXIT_FAILURE);
    }
}

