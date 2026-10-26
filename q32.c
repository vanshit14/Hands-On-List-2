/*
 * Q32: Send data from parent to child using an anonymous pipe.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }
    // After fork, both processes inherit the pipe descriptors, so they can use the same pipe.
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return 1;
    }
    // fork returns zero in the child. This branch receives the parent's message.
    if (pid == 0)
    {
        char buffer[128];
        // The child does not send anything, so it closes its copy of the write end.
        close(fd[1]); /* Child only reads. */
        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n == -1)
        {
            perror("read");
            return 1;
        }
        buffer[n] = '\0';
        printf("Child received: %s\n", buffer);
        close(fd[0]);
        return 0;
    }
    // Only the parent reaches this part. It keeps the write end and sends the message.
    close(fd[0]); /* Parent only writes. */
    const char message[] = "Hello child, from your parent!";
    if (write(fd[1], message, sizeof(message) - 1) == -1)
    {
        perror("write");
        return 1;
    }
    close(fd[1]);
    int status;
    // Wait for the child to finish reading and report whether it ended successfully.
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
