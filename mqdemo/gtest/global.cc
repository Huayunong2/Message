#include <iostream>
#include <gtest/gtest.h>

class MyEnvironment : public testing::Environment
{
public:
    virtual void SetUp() override
    {
        std::cout << "单元测试执行前的环境初始化\n";
    }

    virtual void TearDown() override
    {
        std::cout << "单元测试执行后的环境清理\n";
    }
};

TEST(MyEnvironment, test1)
{
    std::cout << "单元测试1\n";
}

TEST(MyEnvironment, test2)
{
    std::cout << "单元测试2\n";
}

int main(int argc, char *argv[])
{
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}