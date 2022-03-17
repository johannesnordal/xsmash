#include "common.h"
#include <time.h>

void process(int sockfd)
{
    int size = rand() % 10;
    if (write(sockfd, &size, sizeof(int)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    int buffer[size];
    for (int i = 0; i < size; i++)
        buffer[i] = rand() % 10;

    if (write(sockfd, &buffer, sizeof(buffer)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    int sum;
    if (read(sockfd, &sum, sizeof(sum)) == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("%d\n", sum);
}

int main(int argc, char **argv)
{
    time_t t;
    srand((unsigned) time(&t));

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    process(sockfd);
    close(sockfd);
}
