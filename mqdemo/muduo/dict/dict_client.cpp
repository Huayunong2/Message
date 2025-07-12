#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/CountDownLatch.h>
#include <iostream>
#include <functional>

class TranslateClient
{
public:
    TranslateClient(const std::string &sip, int sport)
        : _latch(1),
          _client(_loopthread.startLoop(), muduo::net::InetAddress(sip, sport), "TranslateClient")
    {
        _client.setConnectionCallback(std::bind(&TranslateClient::onConnection, this, std::placeholders::_1));
        _client.setMessageCallback(std::bind(&TranslateClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    // 需要阻塞等待连接建立成功后再返回
    void connect()
    {
        _client.connect();
        _latch.wait(); // 阻塞等待，直到链接建立成功
    }
    bool send(const std::string &msg)
    {
        // 链接状态正常，才发送消息
        if (_conn->connected())
        {
            _conn->send(msg);
            return true;
        }
        return false;
    }

private:
    // 新链接建立成功时的回调函数，链接建立成功后，唤醒上边的阻塞
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            _latch.countDown(); // 唤醒主线程的阻塞
            _conn = conn;
        }
        else
        {
            _conn.reset();
            std::cout << "链接关闭!\n";
        }
    }

    // 通信连接收到请求时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp timestamp)
    {
        std::cout << "翻译结果:" << buf->retrieveAllAsString() << std::endl;
    }

private:
    muduo::CountDownLatch _latch;            // 先进行阻塞，保证一定能建立连接成功
    muduo::net::EventLoopThread _loopthread; // 这样可以自己创建线程进行操作
    muduo::net::TcpClient _client;
    muduo::net::TcpConnectionPtr _conn;
};

int main()
{
    TranslateClient client("127.0.0.1", 8888);
    client.connect();
    while (1)
    {
        std::string buf;
        std::cin >> buf;
        client.send(buf);
    }
    return 0;
}
