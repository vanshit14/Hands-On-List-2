/*
 * Q49(a): Protect ticket-file updates with a binary semaphore.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <fcntl.h>

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
    /* One run launches four cooperating booking processes.
       Use a fresh file per run: independent runs do not share this semaphore. */
    char path[] = "q49_tickets_XXXXXX";
    // Open a separate ticket file for this run so old ticket data cannot affect the example.
    int fd = mkstemp(path), ticket = 0;
    if (fd == -1)
    {
        perror("mkstemp");
        return 1;
    }
    if (write(fd, &ticket, sizeof(ticket)) != sizeof(ticket))
    {
        perror("initialize ticket");
        close(fd);
        unlink(path);
        return 1;
    }
    // One available slot means only one booking process can update the ticket file at a time.
    int id = make_sem(1);
    if (id == -1)
    {
        close(fd);
        unlink(path);
        return 1;
    }
    printf("Ticket file: %s\n", path);
    fflush(stdout);
    pid_t children[4];
    int count = 0, result = 0;
    // Start four booking processes that all use the same file and semaphore.
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
            /* Critical section is the whole read-modify-write transaction.
               pread/pwrite avoid the inherited descriptor's shared offset. */
            // Lock the ticket update before reading the current number.
            if (change(id, -1) == -1)
            {
                _exit(1);
            }
            if (pread(fd, &ticket, sizeof(ticket), 0) != sizeof(ticket))
            {
                perror("pread");
                _exit(1);
            }
            // The next ticket is one greater than the last stored ticket.
            ++ticket;
            if (pwrite(fd, &ticket, sizeof(ticket), 0) != sizeof(ticket))
            {
                perror("pwrite");
                _exit(1);
            }
            printf("Process %ld booked ticket %d\n", (long)getpid(), ticket);
            fflush(stdout);
            // Release the lock only after the updated ticket has been written.
            if (change(id, 1) == -1)
            {
                _exit(1);
            }
            close(fd);
            _exit(0);
        }
        children[count++] = pid;
    }
    // Wait for every booking to finish, then remove the semaphore.
    result = finish(id, children, count, result);
    close(fd);
    if (unlink(path) == -1)
    {
        perror("unlink");
        result = 1;
    }
    return result;
}
