#include "../mqserver/mq_binding.hpp"
#include <gtest/gtest.h>

MQ::BindingManager::ptr bmp;

class BindingTest : public testing::Environment
{
public:
    virtual void SetUp() override
    {
        bmp = std::make_shared<MQ::BindingManager>("./data/meta.db");
    }

    virtual void TearDown() override
    {
        // emp->clear();
        std::cout << "最后的清理\n";
    }
};

TEST(binding_test, insert_test)
{
    bmp->bind("exchange1", "queue1", "news.music.#", true);
    bmp->bind("exchange1", "queue2", "news.sport.#", true);
    bmp->bind("exchange1", "queue3", "news.gossip.#", true);

    bmp->bind("exchange2", "queue1", "news.music.pop", true);
    bmp->bind("exchange2", "queue2", "news.sport.footboll", true);
    bmp->bind("exchange2", "queue3", "news.gossip.player", true);

    ASSERT_EQ(bmp->size(), 6);
}


TEST(binding_test, select_test)
{
    ASSERT_EQ(bmp->exists("exchange1", "queue1"), true);
    ASSERT_EQ(bmp->exists("exchange1", "queue2"), true);
    ASSERT_EQ(bmp->exists("exchange1", "queue3"), true);
    ASSERT_EQ(bmp->exists("exchange2", "queue1"), true);
    ASSERT_EQ(bmp->exists("exchange2", "queue2"), true);
    ASSERT_EQ(bmp->exists("exchange2", "queue3"), true);

    MQ::Binding::ptr bp = bmp->getBinding("exchange1", "queue1");
    ASSERT_NE(bp.get(), nullptr);
    ASSERT_EQ(bp->exchange_name, std::string("exchange1"));
    ASSERT_EQ(bp->msgqueue_name, std::string("queue1"));
    ASSERT_EQ(bp->binding_key, std::string("news.music.#"));
}

TEST(binding_test, select_exchange_test)
{
    MQ::MsgQueueBindingMap mqbm = bmp->getExchangeBindings("exchange1");
    ASSERT_EQ(mqbm.size(), 3);
    ASSERT_NE(mqbm.find("queue1"), mqbm.end());
    ASSERT_NE(mqbm.find("queue2"), mqbm.end());
    ASSERT_NE(mqbm.find("queue3"), mqbm.end());
}

TEST(binding_test, remove_queue_test)
{
    bmp->removeMsgQueueBindings("queue1");
    ASSERT_EQ(bmp->exists("exchange1", "queue1"), false);
    ASSERT_EQ(bmp->exists("exchange2", "queue1"), false);
}

TEST(binding_test, remove_exchange_test)
{
    bmp->removeExchangeBindings("exchange1");
    ASSERT_EQ(bmp->exists("exchange1", "queue1"), false);
    ASSERT_EQ(bmp->exists("exchange1", "queue2"), false);
    ASSERT_EQ(bmp->exists("exchange1", "queue3"), false);
}

TEST(binding_test, remove_single_test)
{
    ASSERT_EQ(bmp->exists("exchange2", "queue2"), true);
    bmp->unBind("exchange2", "queue2");
    ASSERT_EQ(bmp->exists("exchange2", "queue2"), false);
    ASSERT_EQ(bmp->exists("exchange2", "queue3"), true);
}


TEST(binding_test, recovery_test)
{
    ASSERT_EQ(bmp->exists("exchange1", "queue2"), false);
    ASSERT_EQ(bmp->exists("exchange2", "queue1"), false);
    ASSERT_EQ(bmp->exists("exchange2", "queue3"), true);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BindingTest);
    return RUN_ALL_TESTS();
}