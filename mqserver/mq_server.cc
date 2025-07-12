#include "mq_broker.hpp"

int main()
{
    MQ::Server server(8888, "./data/");
    server.start();
    return 0;
}