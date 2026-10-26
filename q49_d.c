/*
 * Q49(d): Remove a semaphore set by ID (for example, one printed by q48).
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <limits.h>

/* glibc requires the application to define union semun. */
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(int argc, char *argv[])
{
    // The semaphore ID is supplied on the command line, using the value printed by q48.
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s semaphore-id\n", argv[0]);
        return 1;
    }
    char *end;
    errno = 0;

    // Convert the ID to a number and check that the whole argument was valid.
    long id = strtol(argv[1], &end, 10);
    if (errno || end == argv[1] || *end || id < 0 || id > INT_MAX)
    {
        fputs("Invalid semaphore ID.\n", stderr);
        return 1;
    }

    // Remove the entire semaphore set identified by this ID.
    if (semctl((int)id, 0, IPC_RMID) == -1)
    {
        perror("IPC_RMID");
        return 1;
    }

    // The set no longer exists after this call succeeds.
    printf("Removed semaphore set %ld.\n", id);
    return 0;
}
