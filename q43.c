/*
 * Q43: Send one type-1 message to the queue created by q41.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
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
    struct
    {
        long type;
        char text[256];
    } message = {.type = 1};

    // Use the supplied message, or a short default message if none was given.
    const char *text = argc > 1 ? argv[1] : "Hello message queue!";
    // Check the length before copying so the message fits in the fixed-size buffer.
    if (strlen(text) >= sizeof(message.text))
    {
        fputs("Message must be shorter than 256 bytes.\n", stderr);
        return 1;
    }
    // Copy the text after the length check. Its type is 1, which must be positive.
    strcpy(message.text, text);
    // Find the existing queue rather than creating a new one for each send.
    int id = get_queue();
    /* msgsz excludes the leading long message-type field. */
    // Send the text and its null character. With flag 0, a full queue makes the sender wait.
    if (msgsnd(id, &message, strlen(message.text) + 1, 0) == -1)
    {
        perror("msgsnd");
        return 1;
    }
    printf("Sent: %s\nInspect with: ipcs -q\n", message.text);
    return 0;
}
