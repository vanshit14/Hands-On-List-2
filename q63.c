/*
 * Q63: Child sends SIGKILL to its own parent, becoming an orphan.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

int main(void)
{
    /* Save this program's PID before fork: never target a shell or another app. */
    pid_t original_parent = getpid();

    // Use a pipe to stop the child from killing the parent before the parent prints its
    // details.
    int gate[2];
    if (pipe(gate) == -1)
    {
        perror("pipe");
        return 1;
    }

    // The new child initially has this program's process as its parent.
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        return 1;
    }
    if (child == 0)
    {
        close(gate[1]);
        char token;

        // Wait for the parent to send permission to continue.
        if (read(gate[0], &token, 1) != 1)
        {
            return 1;
        }
        close(gate[0]);
        printf("Child PID: %ld, parent before kill: %ld\n", (long)getpid(), (long)getppid());
        fflush(stdout);

        // Send SIGKILL only to the parent PID saved before fork.
        if (kill(original_parent, SIGKILL) == -1)
        {
            perror("kill");
            return 1;
        }
        /* Allow reparenting before checking; adopter can be init or a subreaper,
           so the new PPID is not guaranteed to be 1 (especially under WSL). */
        struct timespec delay =
        {
            .tv_sec = 0,
            .tv_nsec = 10000000
        };
        for (int i = 0; i < 200 && getppid() == original_parent; ++i)
        {
            nanosleep(&delay, NULL);
        }

        // Once the parent has died, the child is adopted by another process.
        printf("Orphan child: new parent PID = %ld\n", (long)getppid());
        return getppid() == original_parent ? 1 : 0;
    }
    close(gate[0]);
    printf("Parent PID: %ld, child PID: %ld. Child will kill this parent.\n", (long)original_parent,
           (long)child);
    fflush(stdout);

    // The parent has printed its message, so the child can now perform the kill.
    if (write(gate[1], "G", 1) != 1)
    {
        perror("write");
        return 1;
    }
    close(gate[1]);
    for (;;)
    {
        pause(); /* SIGKILL cannot be caught, blocked, or ignored. */
    }
}
