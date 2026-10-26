/*
 * Q44(b): Receive a message with IPC_NOWAIT.
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

int main(void)
{
    struct
    {
        long type;
        char text[257];
    } message;

    // Open the assignment queue using the key shared with the sender.
    int id = get_queue();
    /* Type 0 selects the oldest message, irrespective of its type.
       Reserve one byte so even a non-NUL payload can be printed safely. */
    // Try to receive the oldest message immediately. IPC_NOWAIT prevents waiting on an empty
    // queue.
    ssize_t n = msgrcv(id, &message, sizeof(message.text) - 1, 0, IPC_NOWAIT);

    // A negative result means no message was received; errno explains the reason.
    if (n == -1)
    {
        if (errno == ENOMSG)
        {
            puts("No message available; did not block.");
            return 0;
        }
        perror("msgrcv");
        return 1;
    }

    // Add a null character after the received bytes before printing the message.
    message.text[n] = '\0';

    // Display both the message type and its text. Receiving also removes the message from the
    // queue.
    printf("Received type %ld: %s\n", message.type, message.text);
    return 0;
}
