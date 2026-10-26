/*
 * Q37: One-way FIFO writer. Start the reader in another terminal.
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
    // Use the same FIFO name as the reader so the two programs meet at the same pipe.
    ensure_fifo("q37_fifo");
    /* Blocking open waits until a reader opens the FIFO. */
    // Open only the write end. This waits until the reader also opens the FIFO.
    int fd = open("q37_fifo", O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    // This example sends one short message. The reader prints the bytes it receives.
    const char message[] = "Hello from the FIFO writer!\n";
    if (write(fd, message, sizeof(message) - 1) == -1)
    {
        perror("write");
        return 1;
    }

    // Closing the last write end lets the reader know that no more data is coming.
    close(fd);
    puts("Message sent. After both programs exit: rm q37_fifo");
    return 0;
}
