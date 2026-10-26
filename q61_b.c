/*
 * Q61(b): Catch SIGINT using sigaction().
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// This function runs when the registered signal is delivered.
static void handler(int number)
{
    (void)number;
    /* write and _exit are async-signal-safe; printf is not. */
    static const char text[] = "Caught SIGINT using sigaction.\n";
    // Preserve errno so the handler's write does not change an earlier error value.
    int saved = errno;
    // Write the message directly; buffered printing functions are not safe to call in a signal
    // handler.
    (void)write(STDOUT_FILENO, text, sizeof(text) - 1);
    errno = saved;
    // Finish from the handler instead of returning to the interrupted code.
    _exit(0);
}

int main(void)
{
    // Register handler as the function to call for SIGINT.
    struct sigaction action = {0};
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    printf("PID: %ld. Press Ctrl+C or run: kill -INT %ld\n", (long)getpid(), (long)getpid());
    fflush(stdout);
    // Stay alive until Ctrl+C or kill sends SIGINT to this process.
    for (;;)
    {
        pause();
    }
}
