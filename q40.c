/*
 * Q40: Print the process open-file limit, pipe capacity, and PIPE_BUF.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>

int main(void)
{
    struct rlimit limit;
    // Read the limit that controls how many file descriptors this process may use.
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
    {
        perror("getrlimit");
        return 1;
    }
    if (limit.rlim_cur == RLIM_INFINITY)
    {
        puts("Open-file soft limit: unlimited");
    }
    else
    {
        printf("Open-file soft limit: %llu\n", (unsigned long long)limit.rlim_cur);
    }
    // Create a real pipe so we can ask the kernel about its current buffer size.
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }
    /* Linux F_GETPIPE_SZ returns actual circular-buffer capacity in bytes. */
    // The capacity is how many bytes the pipe can hold before a writer has to wait.
    int capacity = fcntl(fd[0], F_GETPIPE_SZ);
    if (capacity == -1)
    {
        perror("F_GETPIPE_SZ");
        return 1;
    }
    printf("Pipe buffer capacity: %d bytes\n", capacity);
    // Also ask how large a write can be without its bytes mixing with another writer's write.
    long atomic = fpathconf(fd[0], _PC_PIPE_BUF);
    if (atomic == -1)
    {
        perror("fpathconf");
        return 1;
    }
    /* PIPE_BUF is the atomic-write limit, not the total pipe capacity. */
    printf("PIPE_BUF (atomic write limit): %ld bytes\n", atomic);
    // Neither end is needed after the measurements, so release both descriptors.
    close(fd[0]);
    close(fd[1]);
    return 0;
}
