/*
 * Q59(d): Catch SIGALRM with signal(), triggered by alarm.
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
// Share a small flag between the handler and the main program.
static volatile sig_atomic_t received = 0;

// The handler only sets the flag. The main program prints after the handler returns.
static void handler(int number)
{
    (void)number;
    received = 1;
}

int main(void)
{
    // Connect SIGALRM to our handler before starting the timer.
    if (signal(SIGALRM, handler) == SIG_ERR)
    {
        perror("signal");
        return 1;
    }
    // Start a one-second wall-clock timer. It sends SIGALRM when it expires.
    alarm(1); /* One-shot wall-clock timer, in seconds. */
    /* Busy work advances virtual/profiling CPU timers. Testing a flag avoids
       // Keep executing until the signal handler records that the timer has expired.
       the race in while (!received) pause() if a signal arrives before pause. */
    while (!received)
    {
    }
    // The flag is now set, so the timer signal was handled successfully.
    puts("Caught SIGALRM using signal().");
    return 0;
}
