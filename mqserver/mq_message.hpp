#ifndef __M_MSG_H__
#define __M_MSG_H__
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_msg.pb.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <list>

namespace MQ
{
#define DATAFILE_SUBFIX ".mqd"
#define TMPFILE_SUBFIX ".mqd.tmp"
    using MessagePtr = std::shared_ptr<MQ::Message>;
    class MessageMapper
    {
    public:
        MessageMapper(std::string &basedir, const std::string &qname)
            : _qname(qname)
        {
            if (basedir.back() != '/')
                basedir.push_back('/');
            _datafile = basedir + qname + DATAFILE_SUBFIX;
            _tmpfile = basedir + qname + TMPFILE_SUBFIX;
            if (FileHelper::createDirectory(basedir) == false)
            {
                assert(FileHelper::createDirectory(basedir));
            }
            createMsgFile();
        }

        bool createMsgFile()
        {
            if (FileHelper(_datafile).exists())
            {
                DLOG("队列数据文件 %s 已经存在!", _datafile.c_str());
                return true; // 文件已经存在
            }
            bool ret = FileHelper::createFile(_datafile);
            if (ret == false)
            {
                DLOG("创建队列数据文件 %s 失败!", _datafile.c_str());
                return false;
            }
            return true;
        }

        void removeMsgFile()
        {
            FileHelper::removeFile(_datafile);
            FileHelper::removeFile(_tmpfile);
        }

        bool insert(MessagePtr &msg)
        {
            return insert(_datafile, msg);
        }

        bool remove(MessagePtr &msg)
        {
            // 1.将msg中的有效标志位修改为"0"
            msg->mutable_payload()->set_valid("0");
            // 2.对msg进行序列化
            std::string body = msg->payload().SerializeAsString();
            if (body.size() != msg->length())
            {
                ELOG("消息序列化后长度不一致, 所以无法修改!");
                return false;
            }
            // 3.将序列化后的消息，写入到数据在文件中的指定位置(覆盖原有数据)
            FileHelper helper(_datafile);
            bool ret = helper.write(body.c_str(), msg->offset(), body.size());
            if (ret == false)
            {
                DLOG("写入消息到队列 %s 失败!", _datafile.c_str());
                return false;
            }
            return true;
        }

        std::list<MessagePtr> gc()
        {
            bool ret;
            std::list<MessagePtr> result;
            ret = load(result);
            if (ret == false)
            {
                ELOG("加载消息文件 %s 失败!", _datafile.c_str());
                return result;
            }
            // 2.将有效数据进行序列化存储到临时文件中
            FileHelper::createFile(_tmpfile);
            for (auto &msg : result)
            {
                DLOG("将消息 %s 写入临时文件", msg->payload().body().c_str());
                ret = insert(_tmpfile, msg);
                if (ret == false)
                {
                    ELOG("将消息 %s 写入临时文件 %s 失败!", msg->payload().valid().c_str(), _tmpfile.c_str());
                    return result; // 失败就返回空
                }
            }
            // 3.删除源文件
            ret = FileHelper::removeFile(_datafile);
            if (ret == false)
            {
                ELOG("删除源文件 %s 失败!", _datafile.c_str());
                return result;
            }
            // 4.将临时文件重命名为源文件
            ret = FileHelper(_tmpfile).rename(_datafile);
            if (ret == false)
            {
                ELOG("重命名临时文件为源文件失败!");
                return result;
            }
            // 5.返回新的有效数据
            return result;
        }

    private:
        bool load(std::list<MessagePtr> &result)
        {
            // 1.加载出文件中所有的有效数据; 存储格式 4字节长度|数据
            FileHelper data_file_helper(_datafile);
            size_t offset = 0, msg_size;
            size_t fsize = data_file_helper.size();
            // DLOG("加载消息文件 %s, 文件大小: %zu 字节", _datafile.c_str(), fsize);
            bool ret;
            while (offset < fsize)
            {
                ret = data_file_helper.read((char *)&msg_size, offset, sizeof(size_t));
                if (ret == false)
                {
                    ELOG("读取消息长度失败!");
                    return false;
                }
                offset += sizeof(size_t);
                std::string msg_body(msg_size, '\0');
                ret = data_file_helper.read(&msg_body[0], offset, msg_size);
                if (ret == false)
                {
                    ELOG("读取消息内容失败!");
                    return false;
                }
                offset += msg_size;
                MessagePtr msgp = std::make_shared<Message>();
                msgp->mutable_payload()->ParseFromString(msg_body);
                // 如果是无效消息，就处理下一个，否则保存起来
                if (msgp->payload().valid() == "0")
                {
                    DLOG("消息 %s 无效，跳过!", msgp->payload().body().c_str());
                    continue;
                }
                result.push_back(msgp);
            }
            return true;
        }

        bool insert(const std::string &filename, MessagePtr &msg)
        {
            // 添加在文件末尾
            // 1.进行消息序列化，获取到格式化后的消息
            std::string body = msg->payload().SerializeAsString();
            // 2.获取文件长度
            FileHelper helper(filename);
            size_t fsize = helper.size();
            size_t msg_size = body.size();
            // 3.将数据写入文件的指定位置(先写4字节，再写指定长度)
            bool ret = helper.write((char *)&msg_size, fsize, sizeof(size_t));
            if (ret == false)
            {
                DLOG("写入消息到队列 %s 失败!", _datafile.c_str());
                return false;
            }
            ret = helper.write(body.c_str(), fsize + sizeof(size_t), body.size());
            if (ret == false)
            {
                DLOG("写入消息到队列 %s 失败!", _datafile.c_str());
                return false;
            }
            // 4.更新msg中的实际存储信息
            msg->set_offset(fsize + sizeof(size_t)); // 设置消息在文件中的偏移量
            msg->set_length(body.size());
            return true;
        }

    private:
        std::string _qname;
        std::string _datafile;
        std::string _tmpfile;
    };

    class QueueMessage
    {
    public:
        using ptr = std::shared_ptr<QueueMessage>;
        QueueMessage(std::string &basedir, const std::string &qname)
            : _qname(qname),
              _valid_count(0),
              _total_count(0),
              _mapper(basedir, qname)
        {
        }

        // 单独列出一个函数而不是放在构造函数中，保证不会发生锁冲突
        bool recovery()
        {
            // 恢复历史消息
            std::unique_lock<std::mutex> lock(_mutex);
            _msgs = _mapper.gc();
            for (auto &msg : _msgs)
            {
                _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
            }
            _valid_count = _total_count = _msgs.size();
            return true;
        }

        bool insert(const BasicProperties *bp, const std::string &body, bool queue_is_durable)
        {
            // 1.构造消息对象
            MessagePtr msg = std::make_shared<Message>();
            msg->mutable_payload()->set_body(body);
            if (bp != nullptr)
            {
                DeliveryMode mode = queue_is_durable ? bp->delivery_mode() : DeliveryMode::UNDURABLE;
                msg->mutable_payload()->mutable_properties()->set_id(bp->id());
                msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
                msg->mutable_payload()->mutable_properties()->set_routing_key(bp->routing_key());
            }
            else
            {
                DeliveryMode mode = queue_is_durable ? DeliveryMode::DURABLE : DeliveryMode::UNDURABLE;
                msg->mutable_payload()->mutable_properties()->set_id(UUIDHelper::uuid());
                msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
                msg->mutable_payload()->mutable_properties()->set_routing_key("");
            }

            std::unique_lock<std::mutex> lock(_mutex);
            // 2.判断是否需要持久化
            if (msg->payload().properties().delivery_mode() == DeliveryMode::DURABLE)
            {
                msg->mutable_payload()->set_valid("1");
                // 3.进行持久化存储
                bool ret = _mapper.insert(msg);
                if (ret == false)
                {
                    DLOG("持久化消息 %s 失败!", body.c_str());
                    return false;
                }
                _valid_count++; // 有效消息数量+1
                _total_count++; // 总消息数量+1
                _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
            }
            // 4.内存管理
            _msgs.push_back(msg);
            return true;
        }

        MessagePtr front()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_msgs.empty())
            {
                DLOG("队列 %s 中没有消息!", _qname.c_str());
                return MessagePtr(); // 队列中没有消息
            }
            // 获取队列头部的消息，从_msgs中获取
            MessagePtr msg = _msgs.front();
            _msgs.pop_front(); // 弹出头部消息
            // 将该消息对象，向待确认的hash表中添加一份，等到收到消息确认后删除
            _waitack_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
            return msg;
        }

        // 每次删除消息后，判断是否需要垃圾回收
        bool remove(const std::string &msg_id)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 1.从待确认消息中查找消息
            auto it = _waitack_msgs.find(msg_id);
            if (it == _waitack_msgs.end())
            {
                DLOG("待确认消息中没有找到消息 %s!", msg_id.c_str());
                return true;
            }
            // 2.如果找到，决定是否删除该消息
            if (it->second->payload().properties().delivery_mode() == DeliveryMode::DURABLE)
            {
                // 3.删除持久化信息
                _mapper.remove(it->second);
                _durable_msgs.erase(msg_id);
                _valid_count--; // 有效消息数量-1
                gc();           // 判断是否需要进行垃圾回收
            }
            // 4.删除待确认消息
            _waitack_msgs.erase(msg_id);
            DLOG("删除消息 %s 成功!", msg_id.c_str());
            return true;
        }

        size_t getable_count()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 返回当前队列中可以被消费的消息数量
            return _msgs.size();
        }

        size_t total_count()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 返回当前队列中可以被消费的消息数量
            return _total_count;
        }

        size_t durable_count()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 返回当前队列中可以被消费的消息数量
            return _durable_msgs.size();
        }

        size_t waitack_count()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 返回当前队列中待确认的消息数量
            return _waitack_msgs.size();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _mapper.removeMsgFile();         // 删除消息文件
            _msgs.clear();                   // 清空待推送消息链表
            _durable_msgs.clear();           // 清空持久化消息链表
            _waitack_msgs.clear();           // 清空待确认消息链表
            _valid_count = _total_count = 0; // 重置消息数量
        }

    private:
        bool GCCheck()
        {
            // 消息总量>2000,有效比例<50%则需要垃圾回收
            if (_total_count > 2000 && _valid_count * 10 / _total_count < 5)
            {
                return true;
            }
            return false;
        }

        void gc()
        {
            // 1.获取垃圾回收后，有效的消息信息链表
            if (GCCheck() == false)
                return;
            std::list<MessagePtr> msgs = _mapper.gc();
            for (auto &msg : msgs)
            {
                auto it = _durable_msgs.find(msg->payload().properties().id());
                if (it == _durable_msgs.end())
                {
                    DLOG("垃圾回收后，有一条持久化消息，在内存中没有进行管理!");
                    _msgs.push_back(msg); // 重新添加到推送链表的末尾
                    _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
                    continue;
                }
                // 2.更新每一条消息的实际存储位置
                it->second->set_offset(msg->offset());
                it->second->set_length(msg->length());
            }
            // 3.更新消息数量
            _valid_count = _total_count = msgs.size();
        }

    private:
        std::mutex _mutex;
        std::string _qname;
        size_t _valid_count;
        size_t _total_count;
        MessageMapper _mapper;
        std::list<MessagePtr> _msgs;                               // 待推送消息
        std::unordered_map<std::string, MessagePtr> _durable_msgs; // 持久化消息
        std::unordered_map<std::string, MessagePtr> _waitack_msgs; // 待确认消息
    };

    class MessageManager
    {
    public:
        using ptr = std::shared_ptr<MessageManager>;
        MessageManager(const std::string &basedir)
            : _basedir(basedir)
        {
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (auto &qmsg : _queue_msgs)
            {
                qmsg.second->clear();
            }
        }

        void initQueueMessage(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it != _queue_msgs.end())
                {
                    return;
                }
                qmp = std::make_shared<QueueMessage>(_basedir, qname);
                _queue_msgs.insert(std::make_pair(qname, qmp));
            }
            qmp->recovery();
        }

        void destroyQueueMessage(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                // 防止锁冲突
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    return;
                }
                qmp = it->second;
                _queue_msgs.erase(it);
            }
            qmp->clear();
        }

        bool insert(const std::string &qname, BasicProperties *bp, const std::string &body, bool queue_is_durable)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("队列 %s 不存在，增加消息失败!", qname.c_str());
                    return false;
                }
                qmp = it->second;
            }
            return qmp->insert(bp, body, queue_is_durable);
        }

        MessagePtr front(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("队列 %s 不存在，获取队首消息失败!", qname.c_str());
                    return MessagePtr(); // 返回空指针
                }
                qmp = it->second;
            }
            return qmp->front();
        }

        void ack(const std::string &qname, const std::string &msg_id)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("确认队列%s消息%s失败，没有找到队列!", qname.c_str(), msg_id.c_str());
                    return;
                }
                qmp = it->second;
            }
            qmp->remove(msg_id);
            return;
        }

        size_t getable_count(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("确认队列%s待推送消息数量失败，没有找到队列!", qname.c_str());
                    return 0;
                }
                qmp = it->second;
            }
            return qmp->getable_count();
        }

        size_t total_count(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("确认队列%s总持久化消息数量失败，没有找到队列!", qname.c_str());
                    return 0;
                }
                qmp = it->second;
            }
            return qmp->total_count();
        }

        size_t durable_count(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("确认队列%s有效持久化消息数量失败，没有找到队列!", qname.c_str());
                    return 0;
                }
                qmp = it->second;
            }
            return qmp->durable_count();
        }

        size_t waitack_count(const std::string &qname)
        {
            QueueMessage::ptr qmp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end())
                {
                    DLOG("确认队列%s待确认消息数量失败，没有找到队列!", qname.c_str());
                    return 0;
                }
                qmp = it->second;
            }
            return qmp->waitack_count();
        }

    private:
        std::mutex _mutex;
        std::string _basedir;                                           // 基础目录
        std::unordered_map<std::string, QueueMessage::ptr> _queue_msgs; // 队列名称->队列对象
    };
}

#endif