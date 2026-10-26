/*
 * Q59(f): Catch SIGVTALRM with signal(), triggered by setitimer.
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
    // Connect SIGVTALRM to our handler before starting the timer.
    if (signal(SIGVTALRM, handler) == SIG_ERR)
    {
        perror("signal");
        return 1;
    }

    // Start a one-second user CPU timer. It sends SIGVTALRM when it expires.
    struct itimerval timer =
    {
        .it_value =
        {
            .tv_sec = 1,
            .tv_usec = 0
        },
        .it_interval =
        {
            .tv_sec = 0,
            .tv_usec = 0
        }
    };
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        return 1;
    }
    /* Busy work advances virtual/profiling CPU timers. Testing a flag avoids
       the race in while (!received) pause() if a signal arrives before pause. */
    // Keep executing until the signal handler records that the timer has expired.
    while (!received)
    {
    }

    // The flag is now set, so the timer signal was handled successfully.
    puts("Caught SIGVTALRM using signal().");
    return 0;
}
