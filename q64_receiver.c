/*
 * Q64: Show that SIGSTOP cannot be caught, then wait to be stopped.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// This function is included to test registration, but SIGSTOP will never call it.
static void handler(int number)
{
    (void)number;
    static const char text[] = "This SIGSTOP handler cannot run.\n";
    (void)write(STDOUT_FILENO, text, sizeof(text) - 1);
}

int main(void)
{
    errno = 0;
    // Ask to install a handler. The kernel rejects this because SIGSTOP cannot be caught.
    if (signal(SIGSTOP, handler) == SIG_ERR)
    {
        printf("Register SIGSTOP handler: %s (expected failure).\n", strerror(errno));
    }
    else
    {
        fputs("Unexpectedly registered SIGSTOP handler.\n", stderr);
        return 1;
    }
    // Give the sender this PID so it can stop this particular process.
    printf("Receiver PID: %ld\nIn another terminal: ./q64_sender %ld\n", (long)getpid(),
           (long)getpid());
    puts("SIGSTOP suspends me. SIGCONT resumes me; Ctrl+C then ends me.");
    fflush(stdout);
    // Wait here. SIGSTOP suspends the whole process; SIGCONT lets it run again.
    for (;;)
    {
        // After resuming, keep waiting until another signal ends the program.
        pause();
    }
}
