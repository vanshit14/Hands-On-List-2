/*
 * Q41: Create a System V message queue and print its key and ID.
 */
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(void)
{
    // This fixed key is the common name used by q42 to q46 to find this queue.
    key_t key = (key_t)0x563141;
    /* IPC_EXCL prevents accidentally treating an old queue as a new one. */
    // Create a new queue with owner-only access. Do not silently reuse an older queue.
    int id = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    // For example, creation fails if a queue with this key already exists.
    if (id == -1)
    {
        perror("msgget (existing queue? remove it with q46)");
        return 1;
    }
    // The key identifies the queue to other programs; the ID is used in message-queue system
    // calls.
    printf("Key: 0x%lx\nMessage queue ID: %d\n", (unsigned long)key, id);
    puts("Inspect: ipcs -q. Remove after use: ./q46");
    // Leave the queue in place so the later questions can inspect, send, and receive messages.
    return 0;
}
