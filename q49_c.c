/*
 * Q49(c): Counting semaphore limits access to two equivalent resource slots.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

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
    /* A value of 2 allows at most two workers inside the resource pool. */
    // There are two available slots, so two workers can use the resource pool together.
    int id = make_sem(2);
    if (id == -1)
    {
        return 1;
    }
    pid_t children[5];
    int count = 0, result = 0;
    // Start more workers than there are slots so some workers must wait.
    for (int i = 0; i < 5; ++i)
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
            // Reserve one slot. If both are busy, the worker sleeps inside semop until one is
            // released.
            if (change(id, -1) == -1)
            {
                _exit(1);
            }
            printf("Worker %ld acquired one of two slots.\n", (long)getpid());
            fflush(stdout);
            sleep(1); /* Simulated resource use. */
            printf("Worker %ld releases its slot.\n", (long)getpid());
            fflush(stdout);
            // Return the slot after the simulated work is complete.
            if (change(id, 1) == -1)
            {
                _exit(1);
            }
            _exit(0);
        }
        children[count++] = pid;
    }
    // Wait for all five workers before deleting the semaphore.
    return finish(id, children, count, result);
}
