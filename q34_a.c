/*
 * Q34(a): Execute ls -l | wc using dup.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

// Make a standard input or output descriptor refer to the pipe instead of the terminal.
static int redirect(int source, int target)
{
    /* dup/F_DUPFD return the lowest available descriptor; dup2 names it. */
    // Close the standard descriptor first so the new copy can use that descriptor number.
    if (close(target) == -1 || dup(source) != target)
    {
        return -1;
    }
    return 0;
}

int main(void)
{
    // The pipe carries the output of ls to the input of wc.
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }
    // Create the process that will run ls.
    pid_t left = fork();
    if (left == -1)
    {
        perror("fork");
        return 1;
    }
    if (left == 0)
    {
        // Send the ls output into the pipe instead of displaying it on the terminal.
        if (redirect(fd[1], STDOUT_FILENO) == -1)
        {
            perror("redirect stdout");
            _exit(1);
        }
        close(fd[0]);
        close(fd[1]);
        // Replace this child with ls. exec returns only if it fails.
        execlp("ls", "ls", "-l", (char *)NULL);
        perror("exec ls");
        _exit(1);
    }
    // Create a second child for wc so both commands can run at the same time.
    pid_t right = fork();
    if (right == -1)
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(left, NULL, 0);
        return 1;
    }
    if (right == 0)
    {
        // Make wc read from the pipe instead of the keyboard.
        if (redirect(fd[0], STDIN_FILENO) == -1)
        {
            perror("redirect stdin");
            _exit(1);
        }
        close(fd[0]);
        close(fd[1]);
        execlp("wc", "wc", (char *)NULL);
        perror("exec wc");
        _exit(1);
    }
    /* Parent must close its write end so wc can observe EOF. */
    close(fd[0]);
    close(fd[1]);
    // Wait for both commands and check that neither failed.
    int s1, s2;
    if (waitpid(left, &s1, 0) == -1 || waitpid(right, &s2, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }
    return WIFEXITED(s1) && WEXITSTATUS(s1) == 0 && WIFEXITED(s2) && WEXITSTATUS(s2) == 0 ? 0 : 1;
}
