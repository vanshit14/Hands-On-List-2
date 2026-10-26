/*
 * Q36(e): Create a FIFO using mkfifo(3).
 */
#include <stdio.h>
#include <sys/stat.h>

int main(void)
{
    /* Actual permissions also depend on the process umask. */
    // Create a named pipe with read and write permission for its owner.
    if (mkfifo("q36_mkfifo_fifo", 0600) == -1)
    {
        perror("mkfifo");
        return 1;
    }
    // The FIFO can now be opened by a reader and a writer in separate processes.
    puts("Created q36_mkfifo_fifo. Inspect: ls -l q36_mkfifo_fifo");
    puts("Cleanup: rm q36_mkfifo_fifo");
    // Exiting closes the program, but the named pipe remains until it is removed.
    return 0;
}
