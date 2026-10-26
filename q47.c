/*
 * Q47(a-d): Write, attach read-only, demonstrate write failure, detach, remove.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/resource.h>

int main(void)
{
    // Request a new memory segment. IPC_PRIVATE creates a fresh segment for this run.
    int id = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0600);
    if (id == -1)
    {
        perror("shmget");
        return 1;
    }

    // Attach the segment to this process so it can be accessed through a pointer.
    char *memory = shmat(id, NULL, 0);
    if (memory == (void *)-1)
    {
        perror("shmat");
        shmctl(id, IPC_RMID, NULL);
        return 1;
    }

    // Write through the normal attachment before testing a read-only attachment.
    strcpy(memory, "Data written to shared memory.");
    printf("Shared memory ID: %d\nData: %s\n", id, memory);

    // Detach this first mapping; the segment itself still exists in the kernel.
    if (shmdt(memory) == -1)
    {
        perror("shmdt");
        shmctl(id, IPC_RMID, NULL);
        return 1;
    }
    fflush(NULL);

    // Test the invalid write in a child so the parent can check the result and clean up.
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        shmctl(id, IPC_RMID, NULL);
        return 1;
    }
    if (child == 0)
    {
        /* The correct shmat flag is SHM_RDONLY, not open(2)'s O_RDONLY.
           O_RDONLY is usually 0 and would attach writable memory here. */
        // The child can read this attachment, but the memory protection prevents writing to it.
        char *readonly = shmat(id, NULL, SHM_RDONLY);
        if (readonly == (void *)-1)
        {
            perror("shmat read-only");
            _exit(1);
        }
        printf("Read-only attachment reads: %s\n", readonly);
        puts("Attempting overwrite in the child: expect SIGSEGV.");
        fflush(stdout);
        /* Avoid a core file for this deliberately faulting demonstration. */
        struct rlimit no_core =
        {
            0,
            0
        };
        if (setrlimit(RLIMIT_CORE, &no_core) == -1)
        {
            perror("setrlimit");
            _exit(1);
        }
        /* volatile ensures the compiler actually performs the test write. */
        ((volatile char *)readonly)[0] = 'X';
        shmdt(readonly);
        _exit(2); /* Reaching here means the protection did not work. */
    }
    int status, result = 0;

    // Wait for the test child and inspect the signal that ended it.
    if (waitpid(child, &status, 0) == -1)
    {
        perror("waitpid");
        result = 1;
    }
    else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
    {
        puts("Confirmed: overwriting a read-only attachment caused SIGSEGV.");
    }
    else
    {
        fputs("Unexpected child result.\n", stderr);
        result = 1;
    }
    /* Child exit automatically detached its attachment. Parent detached above. */
    // Remove the segment once the test has finished.
    if (shmctl(id, IPC_RMID, NULL) == -1)
    {
        perror("IPC_RMID");
        return 1;
    }
    puts("Shared memory removed.");
    return result;
}
