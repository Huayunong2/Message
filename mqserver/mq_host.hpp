#ifndef __M_HOST_H__
#define __M_HOST_H__
#include "mq_exchange.hpp"
#include "mq_queue.hpp"
#include "mq_message.hpp"
#include "mq_binding.hpp"

namespace MQ
{
    class VirtualHost
    {
    public:
        using ptr = std::shared_ptr<VirtualHost>;
        VirtualHost(const std::string &hname, const std::string &basedir, const std::string &dbfile)
            : _host_name(hname),
              _emp(std::make_shared<ExchangeManager>(dbfile)),
              _mqmp(std::make_shared<MsgQueueManager>(dbfile)),
              _bmp(std::make_shared<BindingManager>(dbfile)),
              _mmp(std::make_shared<MessageManager>(basedir))
        {
            // 获取到所有队列信息，通过队列名称恢复历史消息数据
            QueueMap qm = _mqmp->allQueues();
            for (const auto &q : qm)
            {
                _mmp->initQueueMessage(q.first);
            }
        }

        bool declareExchange(
            const std::string &name,
            ExchangeType type,
            bool durable,
            bool auto_delete,
            const google::protobuf::Map<std::string, std::string> &args)
        {
            return _emp->declareExchange(name, type, durable, auto_delete, args);
        }

        void deleteExchange(const std::string &name)
        {
            // 1.首先删除交换机绑定信息
            _bmp->removeExchangeBindings(name);
            // 2.删除交换机对象
            return _emp->deleteExchange(name);
        }

        bool existsExchange(const std::string &name)
        {
            return _emp->exists(name);
        }

        Exchange::ptr selectExchange(const std::string &ename)
        {
            return _emp->selectExchange(ename);
        }

        bool declareQueue(
            const std::string &qname,
            bool qdurable,
            bool qexclusive,
            bool qauto_delete,
            const google::protobuf::Map<std::string, std::string> &qargs)
        {
            // 初始化队列的消息句柄(消息的存储管理)
            // 队列的创建
            _mmp->initQueueMessage(qname);
            return _mqmp->declareQueue(qname, qdurable, qexclusive, qauto_delete, qargs);
        }

        void deleteQueue(const std::string &qname)
        {
            // 删除的时候队列相关数据：队列的消息，队列的绑定信息
            _mmp->destroyQueueMessage(qname);
            _bmp->removeMsgQueueBindings(qname);
            return _mqmp->deleteQueue(qname);
        }

        bool existsQueue(const std::string &qname)
        {
            return _mqmp->exists(qname);
        }

        QueueMap allQueues()
        {
            return _mqmp->allQueues();
        }

        bool bind(const std::string &ename, const std::string &qname, const std::string &key)
        {
            Exchange::ptr ep = _emp->selectExchange(ename);
            if (ep.get() == nullptr)
            {
                DLOG("交换机 %s 不存在，无法进行绑定!", ename.c_str());
                return false;
            }

            MsgQueue::ptr mqp = _mqmp->selectQueue(qname);
            if (mqp.get() == nullptr)
            {
                DLOG("队列 %s 不存在，无法进行绑定!", qname.c_str());
                return false;
            }

            // 两个都是持久化才进行持久化
            return _bmp->bind(ename, qname, key, ep->durable && mqp->durable);
        }

        void unBind(const std::string &ename, const std::string &qname)
        {
            return _bmp->unBind(ename, qname);
        }

        MsgQueueBindingMap exchangeBindings(const std::string &ename)
        {
            return _bmp->getExchangeBindings(ename);
        }

        bool existsBinding(const std::string &ename, const std::string &qname)
        {
            return _bmp->exists(ename, qname);
        }

        bool basicPublish(const std::string &qname, BasicProperties *bp, const std::string &body)
        {
            MsgQueue::ptr mqp = _mqmp->selectQueue(qname);
            if (mqp.get() == nullptr)
            {
                DLOG("队列 %s 不存在，发布消息失败!", qname.c_str());
                return false;
            }
            return _mmp->insert(qname, bp, body, mqp->durable);
        }

        MessagePtr basicConsume(const std::string &qname)
        {
            return _mmp->front(qname);
        }

        void basicAck(const std::string &qname, const std::string &msg_id)
        {
            return _mmp->ack(qname, msg_id);
        }

        void clear()
        {
            _emp->clear();
            _mqmp->clear();
            _bmp->clear();
            _mmp->clear();
        }

    private:
        std::string _host_name;
        ExchangeManager::ptr _emp;
        MsgQueueManager::ptr _mqmp;
        BindingManager::ptr _bmp;
        MessageManager::ptr _mmp;
    };
}
#endif