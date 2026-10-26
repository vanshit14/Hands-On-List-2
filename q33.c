/*
 * Q33: Two-way parent-child communication requires two pipes.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static int receive(int fd, const char *who)
{
    char buffer[128];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n == -1)
    {
        perror("read");
        return -1;
    }

    // Add a null character after the bytes read so printf can display the text safely.
    buffer[n] = '\0';
    printf("%s received: %s\n", who, buffer);
    return 0;
}

int main(void)
{
    // Use a separate pipe for each direction so a process cannot read its own message.
    int to_child[2], to_parent[2];
    if (pipe(to_child) == -1 || pipe(to_parent) == -1)
    {
        perror("pipe");
        return 1;
    }

    // Create the child only after both pipes are ready.
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    // The child first reads the request and then writes a reply through the other pipe.
    if (pid == 0)
    {
        close(to_child[1]);
        close(to_parent[0]);
        if (receive(to_child[0], "Child") == -1)
        {
            return 1;
        }
        const char reply[] = "Hello parent!";
        if (write(to_parent[1], reply, sizeof(reply) - 1) == -1)
        {
            perror("write");
            return 1;
        }
        close(to_child[0]);
        close(to_parent[1]);
        return 0;
    }
    close(to_child[0]);
    close(to_parent[1]);

    // The parent starts the conversation by sending its request.
    const char message[] = "Hello child!";
    if (write(to_child[1], message, sizeof(message) - 1) == -1)
    {
        perror("write");
        return 1;
    }
    close(to_child[1]);
    /* Each side reads after the other side writes: no circular wait. */
    int result = receive(to_parent[0], "Parent");
    close(to_parent[0]);
    int status;

    // Collect the child's exit status after the reply has been received.
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }
    return result == 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}
