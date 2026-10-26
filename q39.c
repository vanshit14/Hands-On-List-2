/*
 * Q39: Wait at most 10 seconds for FIFO data using select.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/select.h>

/* Reuse an existing FIFO, but reject a regular file with the same name. */
// Create the named pipe if needed. If the name already exists, make sure it really is a FIFO.
static void ensure_fifo(const char *path)
{
    if (mkfifo(path, 0600) == -1)
    {
        struct stat st;
        if (errno != EEXIST || lstat(path, &st) == -1 || !S_ISFIFO(st.st_mode))
        {
            perror(path);
            exit(1);
        }
    }
}

int main(void)
{
    ensure_fifo("q39_fifo");
    // Open without waiting for a writer, because select will handle the waiting for us.
    int fd = open("q39_fifo", O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }
    /* Keep a local writer open so absent writers do not produce repeated EOF.
       This descriptor never writes data; an external writer supplies the data. */
    int keepalive = open("q39_fifo", O_WRONLY | O_NONBLOCK);
    if (keepalive == -1)
    {
        perror("open keepalive");
        return 1;
    }
    if (fd >= FD_SETSIZE)
    {
        fputs("Descriptor too large for select\n", stderr);
        return 1;
    }
    // select takes a set of descriptors. Clear it first, then add our FIFO's read end.
    fd_set readers;
    FD_ZERO(&readers);
    FD_SET(fd, &readers);
    // Give select a maximum waiting time of ten seconds.
    struct timeval timeout = {.tv_sec = 10, .tv_usec = 0};
    puts("Waiting 10 seconds. In another terminal: echo hello > q39_fifo");
    fflush(stdout);
    // The first argument is one more than the highest descriptor in the sets.
    int ready = select(fd + 1, &readers, NULL, NULL, &timeout);
    int result = 0;
    if (ready == -1)
    {
        perror("select");
        result = 1;
    }
    // A zero result means the full waiting time passed without readable data.
    else if (ready == 0)
    {
        puts("Timed out: no data within 10 seconds.");
    }
    else
    {
        char buffer[256];
        // The FIFO is ready, so read the message and display the bytes received.
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        if (n == -1)
        {
            perror("read");
            result = 1;
        }
        else
        {
            buffer[n] = '\0';
            printf("Received: %s\n", buffer);
        }
    }
    close(keepalive);
    close(fd);
    /* Cleanup after external writers finish: rm q39_fifo */
    return result;
}
