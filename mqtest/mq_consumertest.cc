#include <gtest/gtest.h>
#include "../mqserver/mq_consumer.hpp"

MQ::ConsumerManager::ptr cmp;

class QueueConsumerTest : public testing::Environment
{
public:
    virtual void SetUp() override
    {
        cmp = std::make_shared<MQ::ConsumerManager>();
        cmp->InitQueueConsumer("queue1");
    }

    virtual void TearDown() override
    {
        // cmp->clear();
        std::cout << "最后的清理\n";
    }
};

void cb(const std::string &tag, const MQ::BasicProperties *bp, const std::string &body)
{
    std::cout << "回调函数被调用，tag: " << tag << ", body: " << body << std::endl;
}

TEST(consumer_test, insert_test)
{
    cmp->create("consumer1", "queue1", false, cb);
    cmp->create("consumer2", "queue1", false, cb);
    cmp->create("consumer3", "queue1", false, cb);

    ASSERT_EQ(cmp->exists("consumer1", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer3", "queue1"), true);
}

TEST(consumer_test, remove_test)
{
    cmp->remove("consumer1", "queue1");

    ASSERT_EQ(cmp->exists("consumer1", "queue1"), false);
    ASSERT_EQ(cmp->exists("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer3", "queue1"), true);
}

TEST(consumer_test,choose_test)
{
    MQ::Consumer::ptr cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer2");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer3");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer2");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new QueueConsumerTest);
    return RUN_ALL_TESTS();
}