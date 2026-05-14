#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "unix_socket_example"
#define CHUNK_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int sockfd, newsockfd;
    socklen_t clilen;

    char buffer[CHUNK_SIZE];

    struct sockaddr_un serv_addr, cli_addr;

    int n;

    /* Create socket */
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    /* Remove old socket file if exists */
    unlink(SOCK_PATH);

    /* Fill server address */
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCK_PATH);

    /* Bind socket */
    if (bind(sockfd,
             (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    /* Listen for client */
    listen(sockfd, 5);

    printf("Server waiting for connection...\n");

    clilen = sizeof(cli_addr);

    /* Accept client */
    newsockfd = accept(sockfd,
                       (struct sockaddr *)&cli_addr,
                       &clilen);

    if (newsockfd < 0)
        error("ERROR on accept");

    printf("Client connected\n");

    /* Read file size first */
    off_t filesize;

    if (read(newsockfd, &filesize, sizeof(filesize)) < 0)
        error("ERROR reading file size");

    printf("Receiving file (%ld bytes)\n\n", filesize);

    off_t total_received = 0;

    /* Receive file data */
    while (total_received < filesize) {

        n = read(newsockfd, buffer, CHUNK_SIZE);

        if (n < 0)
            error("ERROR reading socket");

        if (n == 0)
            break;

        write(STDOUT_FILENO, buffer, n);

        total_received += n;
    }

    printf("\n\nFile transfer complete\n");

    close(newsockfd);
    close(sockfd);

    unlink(SOCK_PATH);

    return 0;
}