/*
 * Q55: Lower this process's soft open-file limit with setrlimit.
 */
#include <stdio.h>
#include <sys/resource.h>

int main(void)
{
    // Keep the original limits and a second copy containing the change we want to make.
    struct rlimit before, after;
    if (getrlimit(RLIMIT_NOFILE, &before) == -1)
    {
        perror("getrlimit");
        return 1;
    }

    // Copy both fields so the hard limit stays unchanged.
    after = before;
    /* Lower only the soft limit; no elevated privilege is required. */
    if (after.rlim_cur == RLIM_INFINITY || after.rlim_cur > 64)
    {
        after.rlim_cur = 64;
    }
    else if (after.rlim_cur > 1)
    {
        after.rlim_cur /= 2;
    }
    printf("Old soft limit: %llu\n", (unsigned long long)before.rlim_cur);

    // Apply the lower soft limit to this process.
    if (setrlimit(RLIMIT_NOFILE, &after) == -1)
    {
        perror("setrlimit");
        return 1;
    }

    // Read the limit again to show that the kernel accepted the change.
    if (getrlimit(RLIMIT_NOFILE, &after) == -1)
    {
        perror("getrlimit");
        return 1;
    }
    printf("New soft limit: %llu\n", (unsigned long long)after.rlim_cur);
    puts("Only this process and its future children inherit this change.");
    return 0;
}
