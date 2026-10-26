/*
 * Q64: Send SIGSTOP to the PID printed by q64_receiver.
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    // Take the receiver's PID as the only command-line argument.
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s receiver-pid\n", argv[0]);
        return 1;
    }
    char *end;
    errno = 0;

    // Read the PID and check for invalid text or a value that cannot fit in a process ID.
    long value = strtol(argv[1], &end, 10);
    /* Reject 0 and negatives: kill treats those as process-group targets. */
    if (errno || end == argv[1] || *end || value <= 1 || value > INT_MAX)
    {
        fputs("Enter a receiver PID greater than 1.\n", stderr);
        return 1;
    }

    // Use the validated value as the target process ID.
    pid_t receiver = (pid_t)value;

    // Ask the kernel to suspend the receiver. The receiver cannot catch or ignore this signal.
    if (kill(receiver, SIGSTOP) == -1)
    {
        perror("kill SIGSTOP");
        return 1;
    }

    // Check the receiver's state to see the effect; no handler message will appear.
    printf("SIGSTOP sent to %ld. Inspect: ps -o pid,stat,comm -p %ld\n", value, value);
    printf("State T means stopped; no SIGSTOP handler ran. Resume: kill -CONT %ld\n", value);
    return 0;
}
