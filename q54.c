/*
 * Q54: Print supported system resource soft and hard limits.
 */
#include <stdio.h>
#include <sys/resource.h>

// Resource limits may be a number or RLIM_INFINITY. Show the unlimited case as text.
static void print_value(rlim_t value)
{
    if (value == RLIM_INFINITY)
    {
        printf("unlimited");
    }
    else
    {
        printf("%llu", (unsigned long long)value);
    }
}

// The soft limit is the active limit. The hard limit restricts how far the soft limit may be
// raised.
int main(void)
{
    struct
    {
        int resource;
        const char *name;
    } limits[] = {
        {RLIMIT_CPU, "CPU time (seconds)"},
        {RLIMIT_FSIZE, "File size (bytes)"},
        {RLIMIT_DATA, "Data segment (bytes)"},
        {RLIMIT_STACK, "Stack (bytes)"},
        {RLIMIT_CORE, "Core file (bytes)"},
        {RLIMIT_RSS, "Resident set (bytes)"},
        {RLIMIT_NPROC, "Processes per real UID"},
        {RLIMIT_NOFILE, "Open file descriptors"},
        {RLIMIT_MEMLOCK, "Locked memory (bytes)"},
        {RLIMIT_AS, "Address space (bytes)"},
#ifdef RLIMIT_LOCKS
        {RLIMIT_LOCKS, "File locks"},
#endif
#ifdef RLIMIT_SIGPENDING
        {RLIMIT_SIGPENDING, "Pending signals"},
#endif
#ifdef RLIMIT_MSGQUEUE
        {RLIMIT_MSGQUEUE, "POSIX message queue bytes"},
#endif
#ifdef RLIMIT_NICE
        {RLIMIT_NICE, "Nice ceiling"},
#endif
#ifdef RLIMIT_RTPRIO
        {RLIMIT_RTPRIO, "Realtime priority"},
#endif
#ifdef RLIMIT_RTTIME
        {RLIMIT_RTTIME, "Realtime CPU (microseconds)"},
#endif
    };
    int result = 0;
    // Read each resource separately because limits use different units and may differ between
    // machines.
    for (size_t i = 0; i < sizeof(limits) / sizeof(limits[0]); ++i)
    {
        // getrlimit fills in rlim_cur for the soft limit and rlim_max for the hard limit.
        struct rlimit value;
        if (getrlimit(limits[i].resource, &value) == -1)
        {
            perror(limits[i].name);
            result = 1;
            continue;
        }
        printf("%-28s soft=", limits[i].name);
        print_value(value.rlim_cur);
        printf(" hard=");
        print_value(value.rlim_max);
        putchar('\n');
    }
    /* Some Linux limits (e.g. RLIMIT_RSS) are reported but not enforced. */
    return result;
}
