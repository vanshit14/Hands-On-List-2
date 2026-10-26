/*
 * Q46: Remove the assignment message queue.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

int main(void)
{
    // Look up the assignment queue using the same key as q41.
    int id = get_queue();

    // Remove the queue from the kernel. Any messages still in it are discarded.
    if (msgctl(id, IPC_RMID, NULL) == -1)
    {
        perror("IPC_RMID");
        return 1;
    }

    // Print the ID so we can see which queue was removed.
    printf("Removed message queue %d.\n", id);
    return 0;
}
