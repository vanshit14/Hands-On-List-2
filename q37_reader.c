/*
 * Q37: One-way FIFO reader. Pair with q37_writer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

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
    // Both programs use this name to access the same named pipe.
    ensure_fifo("q37_fifo");
    // Open the FIFO for reading. The open waits until a writer connects.
    int fd = open("q37_fifo", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }
    char buffer[128];
    ssize_t n;
    // Keep reading until the writer closes its end and read returns zero.
    while ((n = read(fd, buffer, sizeof(buffer))) > 0)
    {
        // Use the number of bytes returned by read; the buffer need not contain a null
        // character.
        if (fwrite(buffer, 1, (size_t)n, stdout) != (size_t)n)
        {
            perror("stdout");
            return 1;
        }
    }
    if (n == -1)
    {
        perror("read");
        return 1;
    }
    // Release our read descriptor after the message has been received.
    close(fd);
    return 0;
}
