#include "../mqserver/mq_router.hpp"
#include <gtest/gtest.h>

class RouterTest : public testing::Environment
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        // mamp->clear();
        std::cout << "最后的清理\n";
    }
};

TEST(router_test, legal_routing_key)
{
    std::string rkey1 = "news.music.pop";
    std::string rkey2 = "news..music.pop";
    std::string rkey3 = "news..,music.pop";
    std::string rkey4 = "news.music_123.pop";
    ASSERT_EQ(MQ::Router::isLegalRoutingKey(rkey1), true);
    ASSERT_EQ(MQ::Router::isLegalRoutingKey(rkey2), true);
    ASSERT_EQ(MQ::Router::isLegalRoutingKey(rkey3), false);
    ASSERT_EQ(MQ::Router::isLegalRoutingKey(rkey4), true);
}

TEST(router_test, legal_binding_key)
{
    std::string bkey1 = "news.music.pop";
    std::string bkey2 = "news.#.music.pop";
    std::string bkey3 = "news.#.*.music.pop"; //
    std::string bkey4 = "news.*.#.music.pop"; //
    std::string bkey5 = "news.*.*.music.pop";
    std::string bkey6 = "news.#.#.music.pop";  //
    std::string bkey7 = "news.,music_123.pop"; //
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey1), true);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey2), true);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey3), false);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey4), false);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey5), true);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey6), false);
    ASSERT_EQ(MQ::Router::isLegalBindingKey(bkey7), false);
}

TEST(router_test, router_test)
{
    std::vector<std::string> bkeys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb",
        "aaa.bbb",
        "aaa.#.bbb",
        "aaa.bbb.#",
        "#.bbb.ccc",
        "aaa.bbb.ccc",
        "aaa.*",
        "aaa.*.bbb",
        "*.aaa.bbb",
        "#",
        "aaa.#",
        "aaa.#",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "#.ccc",
        "#.ccc",
        "aaa.#.ccc.ccc",
        "aaa.#.bbb.*.bbb"};

    std::vector<std::string> rkeys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.ccc",
        "aaa.bbb.ccc",
        "aaa.ccc.bbb",
        "aaa.bbb.ccc.ddd",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.ccc",
        "aaa.bbb.ccc",
        "aaa.aaa.bbb.ccc",
        "ccc",
        "aaa.bbb.ccc",
        "aaa.bbb.ccc.ccc.ccc",
        "aaa.ddd.ccc.bbb.eee.bbb"};

    std::vector<bool> result = {
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true};
    for (int i = 0; i < bkeys.size(); i++)
    {
        ASSERT_EQ(MQ::Router::route(MQ::ExchangeType::TOPIC, rkeys[i], bkeys[i]), result[i]);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new RouterTest);
    return RUN_ALL_TESTS();
}