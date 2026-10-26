/*
 * Q58: Create three threads and print their pthread IDs.
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// This function runs in each new thread. The argument tells it which thread number to print.
static void *worker(void *argument)
{
    // Read the number supplied by main; each thread gets a different array element.
    int index = *(int *)argument;
    /* On Linux/glibc pthread_t is printable as unsigned long.
       pthread IDs are not the same thing as process IDs or kernel TIDs. */
    printf("Thread %d: pthread ID = %lu\n", index, (unsigned long)pthread_self());
    return NULL;
}

int main(void)
{
    // Keep the thread handles so main can join all the threads it creates.
    pthread_t threads[3];
    int indices[3] =
    {
        1,
        2,
        3
    }, created = 0, result = 0;

    // Create three threads. Scheduling decides which one prints first.
    for (int i = 0; i < 3; ++i)
    {
        /* Separate persistent arguments avoid the shared-loop-variable race. */
        int error = pthread_create(&threads[i], NULL, worker, &indices[i]);
        if (error)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            result = 1;
            break;
        }
        ++created;
    }

    // Joining waits for each worker to finish before main returns.
    for (int i = 0; i < created; ++i)
    {
        int error = pthread_join(threads[i], NULL);
        if (error)
        {
            fprintf(stderr, "pthread_join: %s\n", strerror(error));
            result = 1;
        }
    }
    return result;
}
