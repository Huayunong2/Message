#include "dispatcher.h"
#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "request.pb.h"
#include <iostream>
#include <unordered_map>

class Server
{
public:
    typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
    typedef std::shared_ptr<hyn::TranslateRequest> TranslateRequestPtr;
    typedef std::shared_ptr<hyn::TranslateResponse> TranslateResponsetPtr;
    typedef std::shared_ptr<hyn::Addrequest> AddrequestPtr;
    typedef std::shared_ptr<hyn::AddResponse> AddResponsePtr;

    Server(int port)
        : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "Server", muduo::net::TcpServer::kReusePort),
          _dispatcher(std::bind(&Server::onUnknownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
          _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        // 注册请求处理函数
        _dispatcher.registerMessageCallback<hyn::TranslateRequest>(
            std::bind(&Server::onTranslate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _dispatcher.registerMessageCallback<hyn::Addrequest>(
            std::bind(&Server::onAdd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _server.setMessageCallback(
            std::bind(&ProtobufCodec::onMessage, &_codec, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _server.setConnectionCallback(
            std::bind(&Server::onConnection, this, std::placeholders::_1));
    }

    void start()
    {
        _server.start();
        _baseloop.loop();
    }

private:
    std::string Translate(const std::string &str)
    {
        static std::unordered_map<std::string, std::string> dict_map = {
            {"hello", "你好"},
            {"left", "左边"},
            {"左边", "left"},
            {"你好", "hello"}};
        auto it = dict_map.find(str);
        if (it == dict_map.end())
        {
            return "没有这个数据!\n";
        }
        return it->second;
    }

    void onTranslate(const muduo::net::TcpConnectionPtr &conn, const TranslateRequestPtr &message, muduo::Timestamp)
    {
        // 1.提取有效消息
        std::string req_msg = message->msg();
        // 2.翻译
        std::string rsp_msg = Translate(req_msg);
        // 3.组织protobuf响应
        hyn::TranslateResponse resp;
        resp.set_msg(rsp_msg);
        // 4.发送响应(先序列化，在发送)
        _codec.send(conn, resp);
    }

    void onAdd(const muduo::net::TcpConnectionPtr &conn, const AddrequestPtr &message, muduo::Timestamp)
    {
        int num1 = message->num1();
        int num2 = message->num2();
        int result = num1 + num2;
        hyn::AddResponse resp;
        resp.set_result(result);
        _codec.send(conn, resp);
    }

    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO << "新链接建立成功!\n";
        }
        else
        {
            LOG_INFO << "链接即将关闭!\n";
        }
    }

    void onUnknownMessage(const muduo::net::TcpConnectionPtr &conn, const MessagePtr &message, muduo::Timestamp)
    {
        LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
        conn->shutdown();
    }

private:
    muduo::net::EventLoop _baseloop;
    muduo::net::TcpServer _server;  // 服务器对象
    ProtobufDispatcher _dispatcher; // 请求分发器对象，要向其中注册请求处理函数
    ProtobufCodec _codec;           // protobuf协议处理器，针对收到的请求数据进行protobuf协议处理
};

int main()
{
    Server server(8888);
    server.start();
    return 0;
}