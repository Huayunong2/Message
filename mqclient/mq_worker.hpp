#ifndef __M_WORKER_H__
#define __M_WORKER_H__
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_threadpool.hpp"
#include <muduo/net/EventLoopThread.h>
namespace MQ
{
    class AsyncWorker
    {

    public:
        using ptr = std::shared_ptr<AsyncWorker>;
        muduo::net::EventLoopThread loopthread;
        threadpool pool;
    };
}
#endif