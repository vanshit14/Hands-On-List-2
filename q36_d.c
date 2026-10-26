/*
 * Q36(d): Create a FIFO using mknod(2).
 */
#include <stdio.h>
#include <sys/stat.h>

int main(void)
{
    /* S_IFIFO selects a named pipe; device number is unused for FIFOs. */
    // Create the FIFO in the current directory. Permission 0600 allows only its owner to read
    // and write.
    if (mknod("q36_mknod_fifo", S_IFIFO | 0600, 0) == -1)
    {
        perror("mknod");
        return 1;
    }
    // A successful call creates the FIFO's name; it does not put any message into the pipe.
    puts("Created q36_mknod_fifo. Inspect: ls -l q36_mknod_fifo");
    puts("Cleanup: rm q36_mknod_fifo");
    // The FIFO stays in the directory after this program exits.
    return 0;
}
