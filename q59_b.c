/*
 * Q59(b): Catch SIGINT using signal().
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
    static const char text[] = "Caught SIGINT using signal.\n";
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
    if (signal(SIGINT, handler) == SIG_ERR)
    {
        perror("signal");
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
