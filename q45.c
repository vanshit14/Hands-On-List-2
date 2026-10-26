/*
 * Q45: Change queue permissions through msqid_ds and IPC_SET.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
/* All q41-q46 programs share this key. Use ipcs -q to inspect queues. */
#define QUEUE_KEY ((key_t)0x563141)

// Find the queue created in q41. Using the same key lets these separate programs work together.
static int get_queue(void)
{
    int id = msgget(QUEUE_KEY, 0);
    if (id == -1)
    {
        perror("msgget (run q41 first)");
        exit(1);
    }
    return id;
}

int main(int argc, char *argv[])
{
    // Use permission 0640 unless the user provides another octal mode.
    unsigned long mode = 0640;
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [octal-mode]\n", argv[0]);
        return 1;
    }
    // Read permissions in base 8, the same notation used by chmod.
    if (argc == 2)
    {
        char *end;
        errno = 0;
        mode = strtoul(argv[1], &end, 8);
        if (errno || end == argv[1] || *end || mode > 0777)
        {
            fputs("Use an octal mode from 000 to 777.\n", stderr);
            return 1;
        }
    }
    struct msqid_ds info;
    int id = get_queue();
    // Fetch the current settings before changing the permission bits.
    if (msgctl(id, IPC_STAT, &info) == -1)
    {
        perror("IPC_STAT");
        return 1;
    }
    printf("Old mode: %03o\n", info.msg_perm.mode & 0777);
    // Replace the lower permission bits while keeping the rest of the mode field.
    info.msg_perm.mode = (info.msg_perm.mode & ~0777) | mode;
    // Ask the kernel to apply the changed permissions to the existing queue.
    if (msgctl(id, IPC_SET, &info) == -1)
    {
        perror("IPC_SET");
        return 1;
    }
    printf("New mode: %03lo\n", mode);
    return 0;
}
