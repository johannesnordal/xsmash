#include "common.h"
#include "Triangulate.hpp"

void process(int connfd)
{
    int size;
    if (read(connfd, &size, sizeof(size)) == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    int buffer[size];
    if (read(connfd, &buffer, sizeof(buffer)) == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += buffer[i];

    if (write(connfd, &sum, sizeof(sum)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {

    printf("Starting server ...\n");

    auto hash_locator = read_hash_locator(argv[1]);

    printf("Server ready.\n");

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    len = sizeof(cli);

    while (true)
    {
        connfd = accept(sockfd, (struct sockaddr*) &cli, (socklen_t*) &len);
        if (connfd < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        process(connfd);
    }

    close(sockfd);
}
