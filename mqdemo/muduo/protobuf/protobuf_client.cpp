#include "dispatcher.h"
#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/base/CountDownLatch.h>

#include "request.pb.h"
#include <iostream>
#include <unordered_map>
#include <unistd.h>

class Client
{
public:
    typedef std::shared_ptr<hyn::AddResponse> AddResponsePtr;
    typedef std::shared_ptr<hyn::TranslateResponse> TranslateResponsePtr;
    typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

    Client(const std::string &sip, int sport)
        : _latch(1),
          _client(_loopthread.startLoop(), muduo::net::InetAddress(sip, sport), "Client"),
          _dispatcher(std::bind(&Client::onUnknownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
          _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        _dispatcher.registerMessageCallback<hyn::TranslateResponse>(
            std::bind(&Client::onTranslate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _dispatcher.registerMessageCallback<hyn::AddResponse>(
            std::bind(&Client::onAdd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _client.setMessageCallback(
            std::bind(&ProtobufCodec::onMessage, &_codec, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _client.setConnectionCallback(
            std::bind(&Client::onConnection, this, std::placeholders::_1));
    }

    void connect()
    {
        _client.connect();
        _latch.wait();
    }

    void Translate(const std::string &msg)
    {
        hyn::TranslateRequest req;
        req.set_msg(msg);
        send(&req);
    }

    void Add(int num1, int num2)
    {
        hyn::Addrequest req;
        req.set_num1(num1);
        req.set_num2(num2);
        send(&req);
    }

private:
    bool send(const google::protobuf::Message *message) // 修正返回值类型
    {
        if (_conn && _conn->connected())
        {
            _codec.send(_conn, *message); // 修正调用方式
            return true;
        }
        return false;
    }

    void onUnknownMessage(const muduo::net::TcpConnectionPtr &conn, const MessagePtr &message, muduo::Timestamp)
    {
        LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
        conn->shutdown();
    }

    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            _latch.countDown();
            _conn = conn;
        }
        else
        {
            _conn.reset();
        }
    }

    void onTranslate(const muduo::net::TcpConnectionPtr &conn, const TranslateResponsePtr &message, muduo::Timestamp) // 修正参数类型
    {
        std::cout << "翻译结果: " << message->msg() << std::endl;
    }

    void onAdd(const muduo::net::TcpConnectionPtr &conn, const AddResponsePtr &message, muduo::Timestamp) // 修正参数类型
    {
        std::cout << "加法结果: " << message->result() << std::endl; // 显示计算结果
    }

private:
    muduo::CountDownLatch _latch;            // 实现同步
    muduo::net::EventLoopThread _loopthread; // 异步循环处理线程
    muduo::net::TcpClient _client;
    ProtobufDispatcher _dispatcher;     // 请求分发器
    muduo::net::TcpConnectionPtr _conn; // 客户端对应链接
    ProtobufCodec _codec;               // 协议处理器 (修正类型名称)
};

int main()
{
    Client client("127.0.0.1", 8888);
    client.connect();
    // client.connect();  // 注释掉多余的connect()调用

    client.Translate("hello");
    client.Add(11, 22);
    sleep(1);
    return 0;
}