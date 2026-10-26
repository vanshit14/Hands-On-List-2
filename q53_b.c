/*
 * Q53(b): Set ITIMER_VIRTUAL to 10 seconds and 10 microseconds.
 */
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
// Use a flag that the handler can safely change and the main loop can check.
static volatile sig_atomic_t expired = 0;

// Keep the handler short: it only records that the timer expired.
static void on_timer(int number)
{
    (void)number;
    expired = 1;
}

int main(void)
{
    // Install the handler before starting the timer so its first signal is not missed.
    struct sigaction action = {0};
    action.sa_handler = on_timer;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGVTALRM, &action, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    /* First expiration and repeating interval are both 10 s + 10 us.
       Signal delivery is not guaranteed to be precise to a microsecond. */
    // Set both the first delay and the repeat delay. This timer counts time spent running this
    // process in user mode.
    struct itimerval timer = {.it_value = {.tv_sec = 10, .tv_usec = 10},
                              .it_interval = {.tv_sec = 10, .tv_usec = 10}};
    printf("ITIMER_VIRTUAL: waiting for 10 seconds and 10 microseconds.\n");
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        return 1;
    }
    /* CPU timers require CPU work: sleeping would not advance ITIMER_VIRTUAL.
       A busy loop also avoids the flag-check/pause lost-wakeup race. */
    // Keep running until the handler changes the flag.
    while (!expired)
    {
    }
    // A timer filled with zeros turns the interval timer off.
    timer = (struct itimerval){0};
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1)
    {
        perror("disable timer");
        return 1;
    }
    puts("Timer signal received; repeating timer disabled after first tick.");
    return 0;
}
