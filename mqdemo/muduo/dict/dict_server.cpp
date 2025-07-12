#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <iostream>
#include <functional>

class TranslateServer
{
public:
    TranslateServer(int port)
        : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "TranslateServer", muduo::net::TcpServer::kReusePort)
    {
        // 将我们的类成员函数，设置为服务器的回调处理函数
        // bind是一个函数适配器函数，对指定的函数进行参数绑定
        _server.setConnectionCallback(std::bind(&TranslateServer::onConnection, this, std::placeholders::_1)); // this已经被绑定了
        _server.setMessageCallback(std::bind(&TranslateServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    // 开始运行
    void start()
    {
        _server.start();  // 监听
        _baseloop.loop(); // 事件监控，这是一个死循环阻塞接口
    }

private:
    // 新链接建立成功时的回调函数，应该在建立成功和关闭时被调用
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            std::cout << "新链接建立成功!\n";
        }
        else
        {
            std::cout << "新链接关闭!\n";
        }
    }
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
    // 通信连接收到请求时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp timestamp)
    {
        // 1.从buffer中把请求的数据提出
        std::string str = buf->retrieveAllAsString();
        // 2.调用接口进行翻译
        std::string resp = Translate(str);
        // 3.响应结果
        conn->send(resp);
    }

private:
    muduo::net::EventLoop _baseloop; // baseloop是epoll的事件监控，会进行描述符的事件监控，触发事件后进行io操作
    muduo::net::TcpServer _server;   // 通过server告诉服务器该如何处理请求
};

int main()
{
    TranslateServer server(8888);
    server.start();
    return 0;
}
