/*
 * Q38: Two-way FIFO process B; run q38_a and q38_b in separate terminals.
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
    // Create the two named pipes before starting the exchange. Both peers use the same names.
    ensure_fifo("q38_b_to_a");
    ensure_fifo("q38_a_to_b");
    /* Complementary open order avoids both peers waiting as writers. */
    // Open the incoming FIFO to receive the other program's message.
    int in = open("q38_a_to_b", O_RDONLY);
    if (in == -1)
    {
        perror("open reader");
        return 1;
    }
    char buffer[128];

    // Read the peer's message, leaving room for a final null character.
    ssize_t n = read(in, buffer, sizeof(buffer) - 1);
    if (n == -1)
    {
        perror("read");
        return 1;
    }

    // Turn the received bytes into a C string before printing them.
    buffer[n] = '\0';
    printf("Process B received: %s\n", buffer);
    close(in);

    // Open the outgoing FIFO for writing; the other program opens its read end.
    int out = open("q38_b_to_a", O_WRONLY);
    if (out == -1)
    {
        perror("open writer");
        return 1;
    }
    const char message[] = "Hello from process B!";

    // Send one message through the outgoing pipe.
    if (write(out, message, sizeof(message) - 1) == -1)
    {
        perror("write");
        return 1;
    }
    close(out);
    /* Cleanup after BOTH peers finish: rm q38_a_to_b q38_b_to_a */
    return 0;
}
