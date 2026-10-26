/*
 * Q52(a): Concurrent TCP echo server using fork; test with q51_client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Handle short sends and interrupted calls; TCP is a byte stream. */
// One send call may send only part of the data, so keep going until all bytes have been sent.
static int send_all(int fd, const char *data, size_t length)
{
    while (length)
    {
        ssize_t n = send(fd, data, length, MSG_NOSIGNAL);
        if (n == -1 && errno == EINTR)
        {
            continue;
        }
        if (n <= 0)
        {
            return -1;
        }
        data += n;
        length -= (size_t)n;
    }
    return 0;
}

// Read the port from the command line and reject values outside the valid port range.
static int port_number(const char *text)
{
    char *end;
    errno = 0;
    long port = strtol(text, &end, 10);
    if (errno || end == text || *end || port < 1 || port > 65535)
    {
        fputs("Port must be 1..65535.\n", stderr);
        exit(1);
    }
    return (int)port;
}

// Prepare the server socket, give it a local port, and start accepting connection requests.
static int listen_socket(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt");
        close(fd);
        exit(1);
    }
    struct sockaddr_in address =
    {
        .sin_family = AF_INET,
        .sin_port = htons((unsigned short)port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) == -1 || listen(fd, 10) == -1)
    {
        perror("bind/listen");
        close(fd);
        exit(1);
    }
    printf("Listening on port %d (Ctrl+C to stop).\n", port);
    fflush(stdout);
    return fd;
}

// Read data from one client and send the same bytes back. Stop when the client finishes
// sending.
static int serve(int fd)
{
    char buffer[1024];
    for (;;)
    {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n == -1 && errno == EINTR)
        {
            continue;
        }
        if (n == -1)
        {
            perror("recv");
            return 1;
        }
        if (n == 0)
        {
            return 0;
        }
        if (send_all(fd, buffer, (size_t)n) == -1)
        {
            perror("send");
            return 1;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return 1;
    }
    /* SA_NOCLDWAIT avoids accumulating zombie children on Linux. */
    struct sigaction action =
    {
        0
    };
    action.sa_handler = SIG_DFL;

    // Let Linux clean up completed child processes so the server does not leave zombies.
    action.sa_flags = SA_NOCLDWAIT | SA_RESTART;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGCHLD, &action, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    // The parent keeps this listening socket and uses it to accept new clients.
    int listener = listen_socket(argc == 2 ? port_number(argv[1]) : 8080);
    for (;;)
    {
        // Wait for the next incoming connection without affecting children serving earlier
        // clients.
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("accept");
            close(listener);
            return 1;
        }

        // Give the new connection to a child so the parent can accept another client
        // immediately.
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            close(client);
            continue;
        }

        // This child serves just one client and then exits.
        if (pid == 0)
        {
            close(listener); /* Child needs only its connected socket. */
            int result = serve(client);
            close(client);
            _exit(result);
        }
        close(client); /* Parent accepts the next client immediately. */
    }
}
