#include "Sketch.hpp"
#include "Triangulate.hpp"
#include "ServerCommon.hpp"

void process(Sketch& sketch, int sockfd)
{
    size_t size = sketch.min_hash.size();
    if (write(sockfd, &size, sizeof(size_t)) == -1)
        exit_err("write");

    if (write(sockfd, sketch.min_hash.data(), sizeof(uint64_t) * size) == -1)
        exit_err("write");

    Results res[RES_SIZE];
    if (read(sockfd, res, sizeof(res)) == -1)
        exit_err("read");

    for (int i = 0; i < RES_SIZE; ++i)
    {
        if (res[i].genome[0] != '\0')
        {
            std::cout << res[i].id << '\t' << res[i].mutual << '\t';
            std::cout << res[i].genome << '\t' << res[i].atom << '\n';
        }
    }
}

int main(int argc, char **argv)
{
    Sketch sketch = Sketch::read(argv[1]);
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

    process(sketch, sockfd);
    close(sockfd);
}
