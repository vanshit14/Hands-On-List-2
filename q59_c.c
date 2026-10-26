/*
 * Q59(c): Catch SIGFPE using signal().
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
    static const char text[] = "Caught SIGFPE using signal.\n";

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
    // Register handler as the function to call for SIGFPE.
    if (signal(SIGFPE, handler) == SIG_ERR)
    {
        perror("signal");
        return 1;
    }
    puts("Raising SIGFPE deliberately to test the handler.");
    fflush(stdout);
    /* raise is defined and reproducible. A NULL dereference or integer
       division by zero would be undefined C behavior. For a real hardware
       fault, returning without fixing its cause can repeat the same fault. */
    // Send the signal to this process deliberately so the handler can be tested.
    if (raise(SIGFPE) != 0)
    {
        perror("raise");
        return 1;
    }
    return 1; /* Handler should have exited before this point. */
}
