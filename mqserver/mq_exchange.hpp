#ifndef __M_EXCHANGE_H__
#define __M_EXCHANGE_H__
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_msg.pb.h"
#include "../mqcommon/mq_proto.pb.h"
#include <google/protobuf/map.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <string>

namespace MQ
{
    // 1.定义交换机类
    struct Exchange
    {
        using ptr = std::shared_ptr<Exchange>;
        std::string name;
        ExchangeType type;
        bool durable;
        bool auto_delete;
        google::protobuf::Map<std::string, std::string> args; // 其它参数
        Exchange() {}
        Exchange(const std::string &ename, ExchangeType etype, bool edurable, bool eauto_delete, const google::protobuf::Map<std::string, std::string> &eargs)
            : name(ename),
              type(etype),
              durable(edurable),
              auto_delete(eauto_delete),
              args(eargs)
        {
        }

        // args存储键值对，在存储数据库时，会组织一个格式字符串继续努存储 key=value&key=val...
        void setArgs(const std::string &str_args) // 解析str_args，将内容存储到成员中
        {
            std::vector<std::string> sub_args;
            StrHelper::split(str_args, "&", sub_args);

            for (auto &str : sub_args)
            {
                size_t pos = str.find("=");
                std::string key = str.substr(0, pos);
                std::string val = str.substr(pos + 1);
                args[key] = val; // protoc不兼容，只能这么写
            }
        }

        std::string getArgs() // 将args中的内容进行序列化后，返回一个字符串
        {
            std::string result;
            for (auto start = args.begin(); start != args.end(); ++start)
            {
                result += start->first + "=" + start->second + "&";
            }
            return result;
        }
    };
    // 2.定义交换机数据持久化管理类——数据存在数据库中
    using ExchangeMap = std::unordered_map<std::string, Exchange::ptr>;
    class ExchangeMapper
    {
    public:
        ExchangeMapper(const std::string &dbfile)
            : _sql_helper(dbfile)
        {
            std::string path = FileHelper::parentDirectory(dbfile);
            FileHelper::createDirectory(path);
            assert(_sql_helper.open());
            createTable();
        }

        void createTable()
        {
#define CREATE_SQL "create table if not exists exchange_table(\
            name varchar(32) primary key, \
            type int, \
            durable int, \
            auto_delete int, \
            args varchar(128));"
            bool ret = _sql_helper.exec(CREATE_SQL, nullptr, nullptr);
            if (ret == false)
            {
                DLOG("创建交换机数据库表失败!");
                abort(); // 异常退出
            }
        }

        void removeTable()
        {
#define DROP_TABLE "drop table if exists exchange_table;"
            bool ret = _sql_helper.exec(DROP_TABLE, nullptr, nullptr);
            if (ret == false)
            {
                DLOG("删除交换机数据库表失败!");
                abort(); // 异常退出
            }
        }

        bool insert(Exchange::ptr &exp)
        {
            std::stringstream ss;
            ss << "insert into exchange_table values(";
            ss << "'" << exp->name << "', ";
            ss << exp->type << ", ";
            ss << exp->durable << ", ";
            ss << exp->auto_delete << ", ";
            ss << "'" << exp->getArgs() << "');";
            return _sql_helper.exec(ss.str(), nullptr, nullptr);
        }

        void remove(const std::string &name)
        {
            std::stringstream ss;
            ss << "delete from exchange_table where name=";
            ss << "'" << name << "';";
            _sql_helper.exec(ss.str(), nullptr, nullptr);
        }

        ExchangeMap recovery()
        {
            ExchangeMap result;
            std::string sql = "select name, type, durable, auto_delete, args from exchange_table;";
            _sql_helper.exec(sql, selectCallback, &result);
            return result;
        }

    private:
        static int selectCallback(void *arg, int numcol, char **row, char **fields)
        {
            ExchangeMap *result = (ExchangeMap *)arg;
            auto exp = std::make_shared<Exchange>();
            exp->name = row[0];
            exp->type = (MQ::ExchangeType)std::stoi(row[1]);
            exp->durable = (bool)std::stoi(row[2]);
            exp->auto_delete = (bool)std::stoi(row[3]);
            if (row[4])
                exp->setArgs(row[4]);
            result->insert(std::make_pair(exp->name, exp));
            return 0;
        }

    private:
        SqliteHelper _sql_helper;
    };
    // 3.定义交换机数据内存管理类
    class ExchangeManager
    {
    public:
        using ptr = std::shared_ptr<ExchangeManager>;
        ExchangeManager(const std::string &dbfile)
            : _mapper(dbfile)
        {
            _exchanges = _mapper.recovery();
        }
        // 声明交换机
        bool declareExchange(const std::string &name,
                             ExchangeType type, bool durable, bool auto_delete,
                             const google::protobuf::Map<std::string, std::string> &args)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            if (it != _exchanges.end())
            {
                // 如果交换机已经存在，那就直接返回，不需要重复新增。
                return true;
            }
            auto exp = std::make_shared<Exchange>(name, type, durable, auto_delete, args);
            if (durable == true)
            {
                bool ret = _mapper.insert(exp);
                if (!ret)
                    return false;
            }
            _exchanges.insert(std::make_pair(name, exp));
            return true;
        }
        void deleteExchange(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            if (it == _exchanges.end())
            {
                // 如果交换机不存在，就直接返回
                return;
            }
            // 持久化操作才删除
            if (it->second->durable == true)
                _mapper.remove(name);
            _exchanges.erase(name);
        }
        // 获取指定交换机对象
        Exchange::ptr selectExchange(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            if (it == _exchanges.end())
            {
                // 如果交换机不存在，就直接返回
                return Exchange::ptr();
            }
            return it->second;
        }
        bool exists(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            if (it == _exchanges.end())
            {
                // 如果交换机不存在，就直接返回
                return false;
            }
            return true;
        }

        size_t size()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _exchanges.size();
        }
        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _mapper.removeTable();
            _exchanges.clear();
        }

    private:
        std::mutex _mutex;
        ExchangeMapper _mapper;
        ExchangeMap _exchanges;
    };
}
#endif