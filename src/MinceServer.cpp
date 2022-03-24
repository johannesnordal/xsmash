#include "Triangulate.hpp"
#include "common.h"

void process(HashLocator hash_locator, Indices indices, int connfd)
{
    size_t size;
    if (read(connfd, &size, sizeof(size)) == -1)
        exit_err("process: read");

    uint64_t min_hash[size];
    if (read(connfd, min_hash, sizeof(uint64_t) * size) == -1)
        exit_err("process: read");

    Results res[5];
    memset(&res, 0, sizeof(res));
    get_results(res, hash_locator, indices, min_hash, size);
    if (write(connfd, res, sizeof(res)) == -1)
        exit_err("write");
}

int main(int argc, char **argv)
{
    auto hash_locator = read_hash_locator(argv[1]);
    auto indices = read_indices(argv[2]);
    std::cout << "Server ready\n";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        exit_err("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0)
        exit_err("bind");

    if (listen(sockfd, 5) != 0)
        exit_err("listen");

    while (true)
    {
        struct sockaddr_in cli;
        int len = sizeof(cli);
        int connfd = accept(sockfd, (struct sockaddr*) &servaddr, (socklen_t*) &len);
        if (connfd < 0)
            exit_err("accept");
        process(hash_locator, indices, connfd);
    }

    close(sockfd);
}
