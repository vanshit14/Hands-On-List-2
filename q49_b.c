/*
 * Q49(b): Protect a shared-memory counter against concurrent writes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

/* glibc requires the application to define union semun. */
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// A change of -1 takes a semaphore slot; +1 gives it back. A worker waits if no slot is free.
static int change(int id, short delta)
{
    struct sembuf op = {.sem_num = 0, .sem_op = delta, .sem_flg = SEM_UNDO};
    /* SEM_UNDO releases the process's outstanding reservation on exit. */
    while (semop(id, &op, 1) == -1)
    {
        if (errno != EINTR)
        {
            perror("semop");
            return -1;
        }
    }
    return 0;
}

// Create one semaphore and give it the starting number of available slots.
static int make_sem(int value)
{
    int id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (id == -1)
    {
        perror("semget");
        return -1;
    }
    union semun arg = {.val = value};
    if (semctl(id, 0, SETVAL, arg) == -1)
    {
        perror("SETVAL");
        semctl(id, 0, IPC_RMID);
        return -1;
    }
    return id;
}

// Wait for all the workers before removing the semaphore they were using.
static int finish(int id, pid_t *children, int count, int result)
{
    for (int i = 0; i < count; ++i)
    {
        int status;
        if (waitpid(children[i], &status, 0) == -1)
        {
            perror("waitpid");
            result = 1;
        }
        else if (!WIFEXITED(status) || WEXITSTATUS(status))
        {
            result = 1;
        }
    }
    if (semctl(id, 0, IPC_RMID) == -1)
    {
        perror("IPC_RMID");
        result = 1;
    }
    return result;
}

int main(void)
{
    // Allocate one shared integer so every child updates the same counter.
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0600);
    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }
    // Attach the shared integer and access it through counter.
    int *counter = shmat(shmid, NULL, 0);
    if (counter == (void *)-1)
    {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }
    /* Mark now: Linux removes it automatically after the last detach/exit. */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("IPC_RMID");
        shmdt(counter);
        return 1;
    }
    // Start from zero before any worker is created.
    *counter = 0;
    // A binary semaphore allows only one child to update the counter at a time.
    int id = make_sem(1);
    if (id == -1)
    {
        shmdt(counter);
        return 1;
    }
    pid_t children[4];
    int count = 0, result = 0;
    // Create four workers. Each worker adds one to the shared counter a thousand times.
    for (int i = 0; i < 4; ++i)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            result = 1;
            break;
        }
        if (pid == 0)
        {
            for (int j = 0; j < 1000; ++j)
            {
                // Take the lock before changing the shared value.
                if (change(id, -1) == -1)
                {
                    _exit(1);
                }
                ++*counter; /* Read-modify-write must be protected. */
                // Give the lock back so another worker can update the counter.
                if (change(id, 1) == -1)
                {
                    _exit(1);
                }
            }
            if (shmdt(counter) == -1)
            {
                _exit(1);
            }
            _exit(0);
        }
        children[count++] = pid;
    }
    // Wait for all workers before checking the final value.
    result = finish(id, children, count, result);
    // Four successful workers should produce 4000. A missing lock could cause updates to be
    // lost.
    printf("Counter: %d; expected: %d\n", *counter, count * 1000);
    if (*counter != count * 1000)
    {
        result = 1;
    }
    if (shmdt(counter) == -1)
    {
        perror("shmdt");
        result = 1;
    }
    return result;
}
