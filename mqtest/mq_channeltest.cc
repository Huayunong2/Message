#include "../mqserver/mq_channel.hpp"
#include <iostream>
int main()
{
    MQ::ChannelManager::ptr cmp = std::make_shared<MQ::ChannelManager>();
    cmp->openChannel("channel1",
                     std::make_shared<MQ::VirtualHost>("host1", "./data/host1/message/", "./data/host1/host1.db"),
                     std::make_shared<MQ::ConsumerManager>(),
                     MQ::ProtobufCodecPtr(),
                     muduo::net::TcpConnectionPtr(),
                     threadpool::ptr());

    return 0;
}