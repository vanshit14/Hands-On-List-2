/*
 * Q48(b): Create and initialize a counting semaphore.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* glibc requires the application to define union semun. */
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(void)
{
    // Create a new set containing one semaphore. IPC_PRIVATE makes it separate from other runs.
    int id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (id == -1)
    {
        perror("semget");
        return 1;
    }
    // A starting value of 2 represents two available slots, so two users can proceed together.
    union semun arg = {.val = 2};
    // Set the value of semaphore number 0 in this set.
    if (semctl(id, 0, SETVAL, arg) == -1)
    {
        perror("SETVAL");
        semctl(id, 0, IPC_RMID);
        return 1;
    }
    // Save this ID if you want to inspect or remove the semaphore in another program.
    printf("Semaphore ID: %d\nInitial value: %d\n", id, semctl(id, 0, GETVAL));
    printf("Remove after use: ./q49_d %d\n", id);
    // Keep the semaphore after exiting so q49_d can demonstrate removing it.
    return 0;
}
