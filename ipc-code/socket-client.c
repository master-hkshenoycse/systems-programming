#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#define SOCK_PATH "unix_socket_example"
#define CHUNK_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sockfd;
    int fd;
    int n;

    char buffer[CHUNK_SIZE];

    struct sockaddr_un serv_addr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open file */
    fd = open(argv[1], O_RDONLY);

    if (fd < 0)
        error("ERROR opening file");

    /* Create socket */
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    /* Fill server address */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCK_PATH);

    /* Connect to server */
    if (connect(sockfd,
                (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }

    /* Get file size */
    struct stat st;

    if (fstat(fd, &st) < 0)
        error("ERROR getting file size");

    off_t filesize = st.st_size;

    /* Send file size first */
    if (write(sockfd, &filesize, sizeof(filesize)) < 0)
        error("ERROR sending file size");

    printf("Sending file: %s\n", argv[1]);
    printf("File size: %ld bytes\n", filesize);

    /* Read file and send chunks */
    while ((n = read(fd, buffer, CHUNK_SIZE)) > 0) {

        if (write(sockfd, buffer, n) < 0)
            error("ERROR writing to socket");
    }

    if (n < 0)
        error("ERROR reading file");

    printf("File transfer complete\n");

    close(fd);
    close(sockfd);

    return 0;
}