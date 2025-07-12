#ifndef __M_CONSUMER_H__
#define __M_CONSUMER_H__
#include <iostream>
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_msg.pb.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace MQ
{
    using ConsumerCallback = std::function<void(const std::string &, const BasicProperties *bp, const std::string)>;
    struct Consumer
    {
        using ptr = std::shared_ptr<Consumer>;
        std::string tag;           // 消费者标签，用于标识消费者
        std::string qname;         // 订阅的队列名称
        bool auto_ack;             // 自动确认标识
        ConsumerCallback callback; // 消费回调函数

        Consumer()
        {
            DLOG("create consumer");
        }
        Consumer(const std::string &ctag, const std::string &queue_name, bool ack, const ConsumerCallback &cb)
            : tag(ctag), qname(queue_name), auto_ack(ack), callback(cb)
        {
            DLOG("create consumer");
        }
        ~Consumer()
        {
            DLOG("delete consumer");
        }
    };
}
#endif