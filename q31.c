/*
 * Q31: Create a pipe, write a message, and read it back.
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    // The pipe gives us two descriptors: one for reading and one for writing.
    int fd[2];
    char buffer[128];
    const char message[] = "Hello through a pipe!";
    // Create the pipe before trying to send any data through it.
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }
    /* fd[0] is the read end; fd[1] is the write end. */
    // Put the message into the pipe. Do not include the string's final null character.
    if (write(fd[1], message, sizeof(message) - 1) == -1)
    {
        perror("write");
        return 1;
    }
    close(fd[1]); /* Closing the writer lets the reader eventually see EOF. */
    // Leave one byte free in the buffer so we can finish the received text properly.
    ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
    if (n == -1)
    {
        perror("read");
        return 1;
    }
    // read returns a byte count, not a C string. Add the null character ourselves.
    buffer[n] = '\0';
    printf("Read from pipe: %s\n", buffer);
    close(fd[0]);
    return 0;
}
