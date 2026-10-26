/*
 * Q50: Deterministically induce circular-wait deadlock with two semaphores.
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

// Decrease the chosen semaphore by one. If it is already zero, this call waits.
static int take(int id, unsigned short index)
{
    struct sembuf op =
    {
        index,
        -1,
        0
    };
    return semop(id, &op, 1);
}

int main(void)
{
    // Create a set with two semaphores, one for each resource.
    int id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if (id == -1)
    {
        perror("semget");
        return 1;
    }

    // Both resources start free, so each semaphore begins at one.
    unsigned short values[] =
    {
        1,
        1
    };
    union semun arg =
    {
        .array = values
    };
    if (semctl(id, 0, SETALL, arg) == -1)
    {
        perror("SETALL");
        semctl(id, 0, IPC_RMID);
        return 1;
    }

    // Use pipes to make sure both children hold their first resource before requesting the
    // other.
    int ready[2], go[2];
    if (pipe(ready) == -1 || pipe(go) == -1)
    {
        perror("pipe");
        semctl(id, 0, IPC_RMID);
        return 1;
    }
    pid_t children[2];
    int count = 0, result = 0;
    for (int i = 0; i < 2; ++i)
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
            close(ready[0]);
            close(go[1]);

            // Each child takes a different resource first.
            if (take(id, (unsigned short)i) == -1)
            {
                _exit(1);
            }
            printf("Child %d holds semaphore %d.\n", i, i);
            fflush(stdout);

            // Tell the parent that this child's first resource has been acquired.
            if (write(ready[1], "R", 1) != 1)
            {
                _exit(1);
            }
            char token;
            if (read(go[0], &token, 1) != 1)
            {
                _exit(1);
            }
            printf("Child %d now waits for semaphore %d.\n", i, 1 - i);
            fflush(stdout);
            /* Both hold one resource before either requests the other. */
            if (take(id, (unsigned short)(1 - i)) == -1)
            {
                if (errno == EIDRM || errno == EINVAL)
                {
                    _exit(0);
                }
                perror("semop");
                _exit(1);
            }
            _exit(2); /* No child should acquire both in this example. */
        }
        children[count++] = pid;
    }
    close(ready[1]);
    close(go[0]);
    if (count == 2)
    {
        char token;
        for (int i = 0; i < 2; ++i)
        {
            if (read(ready[0], &token, 1) != 1)
            {
                result = 1;
            }
        }

        // Let both children request the second resource only after both are holding the first.
        if (write(go[1], "GG", 2) != 2)
        {
            result = 1;
        }
        sleep(2);
        printf("Blocked waiters: sem0=%d, sem1=%d\n", semctl(id, 0, GETNCNT),
               semctl(id, 1, GETNCNT));
        puts("Circular wait: neither can proceed. Removing the set breaks the deadlock.");
    }
    close(ready[0]);
    close(go[1]);

    // Removing the set wakes the blocked children, allowing this demonstration to finish.
    if (semctl(id, 0, IPC_RMID) == -1)
    {
        perror("IPC_RMID");
        result = 1;
    }
    for (int i = 0; i < count; ++i)
    {
        int status;
        if (waitpid(children[i], &status, 0) == -1)
        {
            result = 1;
        }
        else if (!WIFEXITED(status) || WEXITSTATUS(status))
        {
            result = 1;
        }
    }
    return result;
}
