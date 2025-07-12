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

    // 以队列为单元的消费者管理结构
    class QueueConsumer
    {
    public:
        using ptr = std::shared_ptr<QueueConsumer>;
        QueueConsumer(const std::string &qname)
            : _qname(qname), _rr_seq(0)
        {
        }

        // 创建一个新的消费者
        Consumer::ptr create(const std::string &ctag, const std::string &queue_name, bool ack, const ConsumerCallback &cb)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 判断消费者是否已存在
            for (const auto &consumer : _consumers)
            {
                if (consumer->tag == ctag)
                {
                    // 如果已存在，返回现有消费者
                    return Consumer::ptr();
                }
            }
            // 没有则增
            auto consumer = std::make_shared<Consumer>(ctag, queue_name, ack, cb);
            // 添加管理后返回对象
            _consumers.push_back(consumer);
            return consumer;
        }

        void remove(const std::string &ctag)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 查找并移除指定标签的消费者
            for (auto it = _consumers.begin(); it != _consumers.end(); ++it)
            {
                if ((*it)->tag == ctag)
                {
                    _consumers.erase(it);
                    return; // 找到并移除后退出
                }
            }
            return;
        }

        // RR轮转获取
        Consumer::ptr choose()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_consumers.size() == 0)
            {
                return Consumer::ptr(); // 如果没有消费者，返回空指针
            }
            int idx = _rr_seq % _consumers.size();
            _rr_seq++;
            // 返回当前轮询的消费者
            return _consumers[idx];
        }

        bool empty()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _consumers.size() == 0;
        }

        bool exists(const std::string &ctag)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (auto it = _consumers.begin(); it != _consumers.end(); ++it)
            {
                if ((*it)->tag == ctag)
                {
                    return true;
                }
            }
            return false;
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _consumers.clear();
            _rr_seq = 0;
        }

    private:
        std::string _qname;                    // 队列名称
        std::mutex _mutex;                     // 保护消费者列表的互斥锁
        uint64_t _rr_seq;                      // 轮询序列号
        std::vector<Consumer::ptr> _consumers; // 消费者列表
    };

    class ConsumerManager
    {
    public:
        using ptr = std::shared_ptr<ConsumerManager>;

        ConsumerManager() {}

        void InitQueueConsumer(const std::string &qname)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _qconsumers.find(qname);
            if (it != _qconsumers.end())
            {
                return;
            }

            auto qconsumer = std::make_shared<QueueConsumer>(qname);
            _qconsumers.insert(std::make_pair(qname, qconsumer));
        }

        void destroyQueueConsumer(const std::string &qname)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _qconsumers.erase(qname);
        }

        Consumer::ptr create(const std::string &ctag, const std::string &queue_name, bool ack, const ConsumerCallback &cb)
        {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(queue_name);
                if (it == _qconsumers.end())
                {
                    DLOG("没有找到队列 %s 的消费者管理句柄", queue_name.c_str());
                    return Consumer::ptr(); // 如果队列不存在，返回空指针
                }
                qcp = it->second;
            }
            return qcp->create(ctag, queue_name, ack, cb);
        }

        void remove(const std::string &ctag, const std::string &queue_name)
        {

            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(queue_name);
                if (it == _qconsumers.end())
                {
                    DLOG("没有找到队列 %s 的消费者管理句柄", queue_name.c_str());
                    return;
                }
                qcp = it->second;
            }
            return qcp->remove(ctag);
        }

        Consumer::ptr choose(const std::string &queue_name)
        {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(queue_name);
                if (it == _qconsumers.end())
                {
                    DLOG("没有找到队列 %s 的消费者管理句柄", queue_name.c_str());
                    return Consumer::ptr();
                }
                qcp = it->second;
            }
            return qcp->choose();
        }

        bool empty(const std::string &queue_name)
        {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(queue_name);
                if (it == _qconsumers.end())
                {
                    DLOG("没有找到队列 %s 的消费者管理句柄", queue_name.c_str());
                    return true;
                }
                qcp = it->second;
            }
            return qcp->empty();
        }

        bool exists(const std::string &ctag, const std::string &queue_name)
        {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(queue_name);
                if (it == _qconsumers.end())
                {
                    DLOG("没有找到队列 %s 的消费者管理句柄", queue_name.c_str());
                    return false;
                }
                qcp = it->second;
            }
            return qcp->exists(ctag);
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _qconsumers.clear();
        }

    private:
        std::unordered_map<std::string, QueueConsumer::ptr> _qconsumers;
        std::mutex _mutex;
    };
}
#endif