#include <iostream>
#include <gtest/gtest.h>
#include <unordered_map>

class MyTest : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        std::cout << "所有单元测试前执行，初始化总环境!\n";
        //这里插入公共的测试数据
    }
    static void TearDownTestCase()
    {
        std::cout << "所有单元测试执行完毕，清理总环境!\n";
    }

public:
    std::unordered_map<std::string, std::string> _mymap; //每个单元测试会使用独立的mymap，相对于在同一类中测试，不会相互影响
};

TEST_F(MyTest, insert_test)
{
    _mymap.insert(std::make_pair("hello", "你好"));
    _mymap.insert(std::make_pair("left", "左边"));
}

TEST_F(MyTest, size_test)
{
    ASSERT_EQ(_mymap.size(), 2);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}