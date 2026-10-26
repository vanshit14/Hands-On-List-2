/*
 * Q42: Inspect msqid_ds and its ipc_perm fields.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
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

// Convert a stored time into a readable date. A zero value means that event has not happened
// yet.
static void print_time(const char *label, time_t t)
{
    if (t == 0)
    {
        printf("%s: never\n", label);
    }
    else
    {
        char text[64];
        struct tm *tm = localtime(&t);
        if (tm && strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S %Z", tm))
        {
            printf("%s: %s\n", label, text);
        }
    }
}

int main(void)
{
    // This structure receives the queue's permissions, usage details, times, and process IDs.
    struct msqid_ds info;
    int id = get_queue();
    // IPC_STAT asks the kernel to copy the current queue information into info.
    if (msgctl(id, IPC_STAT, &info) == -1)
    {
        perror("msgctl IPC_STAT");
        return 1;
    }
    printf("Queue ID: %d\nPermissions: %03o\n", id, info.msg_perm.mode & 0777);
    // The owner fields say who owns the queue; the creator fields record who first created it.
    printf("Owner UID: %u, GID: %u\n", (unsigned)info.msg_perm.uid, (unsigned)info.msg_perm.gid);
    printf("Creator UID: %u, GID: %u\n", (unsigned)info.msg_perm.cuid,
           (unsigned)info.msg_perm.cgid);
    // These times show when messages were last sent, received, and when metadata changed.
    print_time("Last send", info.msg_stime);
    print_time("Last receive", info.msg_rtime);
    print_time("Last metadata change", info.msg_ctime);
    /* __msg_cbytes is the Linux/glibc field for bytes currently queued. */
    printf("Current queued bytes: %lu\n", (unsigned long)info.__msg_cbytes);
    // The message count and byte limit describe how much data the queue contains and can hold.
    printf("Number of messages: %lu\n", (unsigned long)info.msg_qnum);
    printf("Maximum allowed bytes: %lu\n", (unsigned long)info.msg_qbytes);
    // These are the processes involved in the most recent send and receive operations.
    printf("Last sender PID: %ld\nLast receiver PID: %ld\n", (long)info.msg_lspid,
           (long)info.msg_lrpid);
    return 0;
}
