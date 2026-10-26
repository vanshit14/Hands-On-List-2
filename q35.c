/*
 * Q35: Count immediate subdirectories with ls -l | grep ^d | wc -l.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    // The first pipe connects ls to grep. The second connects grep to wc.
    int p[2], q[2];
    if (pipe(p) == -1 || pipe(q) == -1)
    {
        perror("pipe");
        return 1;
    }
    pid_t children[3];
    int created = 0, result = 0;
    // Create one child for each command in the pipeline.
    for (int i = 0; i < 3; ++i)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            result = 1;
            break;
        }
        // Each child redirects only the input or output needed by its command.
        if (pid == 0)
        {
            /* Only dup2 is used for descriptor redirection. */
            if ((i == 0 && dup2(p[1], STDOUT_FILENO) == -1) ||
                (i == 1 && (dup2(p[0], STDIN_FILENO) == -1 || dup2(q[1], STDOUT_FILENO) == -1)) ||
                (i == 2 && dup2(q[0], STDIN_FILENO) == -1))
            {
                perror("dup2");
                _exit(1);
            }
            close(p[0]);
            close(p[1]);
            close(q[0]);
            close(q[1]);
            /* -A includes hidden subdirectories, but excludes . and ... */
            if (i == 0)
            {
                execlp("ls", "ls", "-lA", (char *)NULL);
            }
            if (i == 1)
            {
                execlp("grep", "grep", "^d", (char *)NULL);
            }
            if (i == 2)
            {
                execlp("wc", "wc", "-l", (char *)NULL);
            }
            perror("exec");
            _exit(1);
        }
        // Save each child's PID so the parent can wait for every command.
        children[created++] = pid;
    }
    close(p[0]);
    close(p[1]);
    close(q[0]);
    close(q[1]);
    // A pipeline is finished only after all three commands have exited.
    for (int i = 0; i < created; ++i)
    {
        int status;
        if (waitpid(children[i], &status, 0) == -1)
        {
            perror("waitpid");
            result = 1;
        }
        else if (!WIFEXITED(status) ||
                 (WEXITSTATUS(status) != 0 && !(i == 1 && WEXITSTATUS(status) == 1)))
        {
            result = 1; /* grep returns 1 when there are no directories. */
        }
    }
    return result;
}
