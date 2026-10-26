/*
 * Q56: Measure 100 getppid() calls using the x86 timestamp counter.
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#if defined(__i386__) || defined(__x86_64__)
/* LFENCE orders RDTSC relative to surrounding instructions on modern x86.
   The memory clobber also prevents compiler movement across the measurement. */
// Read the processor's timestamp counter before or after the group of calls being measured.
static uint64_t timestamp(void)
{
    unsigned int low, high;
    __asm__ volatile("lfence\n\trdtsc\n\tlfence" : "=a"(low), "=d"(high) : : "memory");
    return ((uint64_t)high << 32) | low;
}

int main(void)
{
    // Keep the returned PID in a volatile variable so the result is actually stored.
    volatile pid_t parent = 0;
    (void)getppid(); /* Warm up lazy symbol resolution outside the measurement. */
    // Take the first counter reading immediately before the loop.
    uint64_t start = timestamp();

    // Call getppid exactly one hundred times inside the measured section.
    for (int i = 0; i < 100; ++i)
    {
        parent = getppid();
    }

    // The difference between the readings gives the elapsed timestamp-counter ticks.
    uint64_t end = timestamp();
    printf("Last parent PID: %ld\n", (long)parent);
    printf("100 getppid calls: %llu TSC ticks\n", (unsigned long long)(end - start));

    // Divide the total by one hundred to estimate the average cost per call.
    printf("Average: %.2f TSC ticks per call\n", (double)(end - start) / 100.0);
    /* Includes loop/counter overhead and scheduling noise. Modern invariant
       TSC ticks are reference-clock ticks, not necessarily CPU core cycles.
       Conversion to seconds requires a separately known TSC frequency. */
    return 0;
}
#else
int main(void)
{
    fputs("This TSC example requires x86/x86-64; use an x86 Linux/WSL host.\n", stderr);
    return 1;
}
#endif
