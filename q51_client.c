/*
 * Q51: TCP client; also works with either concurrent q52 server.
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

int main(int argc, char *argv[])
{
    // The server address is required; the port and message are optional.
    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "Usage: %s server-ipv4 [port] [message]\n", argv[0]);
        return 1;
    }
    int port = argc >= 3 ? port_number(argv[2]) : 8080;
    const char *message = argc == 4 ? argv[3] : "Hello from the client!";
    // AF_INET selects IPv4. htons stores the port in the byte order expected by the network.
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_port = htons((unsigned short)port)};
    // Convert the address from readable text into the binary form used by the socket.
    if (inet_pton(AF_INET, argv[1], &address.sin_addr) != 1)
    {
        fputs("Enter a numeric IPv4 address, e.g. 127.0.0.1.\n", stderr);
        return 1;
    }
    // SOCK_STREAM creates a TCP socket, which provides an ordered stream of bytes.
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return 1;
    }
    // Connect to the listening server before sending the message.
    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("connect");
        close(fd);
        return 1;
    }
    // Send the full request, even if the system accepts it in smaller pieces.
    if (send_all(fd, message, strlen(message)) == -1)
    {
        perror("send");
        close(fd);
        return 1;
    }
    /* Half-close tells the server the request is complete, while allowing reads. */
    if (shutdown(fd, SHUT_WR) == -1)
    {
        perror("shutdown");
        close(fd);
        return 1;
    }
    char buffer[1024];
    fputs("Server echoed: ", stdout);
    for (;;)
    {
        // Read the echoed reply in chunks until the server closes its connection.
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n == -1 && errno == EINTR)
        {
            continue;
        }
        if (n == -1)
        {
            perror("recv");
            close(fd);
            return 1;
        }
        if (n == 0)
        {
            break;
        }
        if (fwrite(buffer, 1, (size_t)n, stdout) != (size_t)n)
        {
            close(fd);
            return 1;
        }
    }
    putchar('\n');
    close(fd);
    return 0;
}
