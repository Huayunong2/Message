#include <iostream>
#include <gtest/gtest.h>


// 断言宏ASSERT_ 失败则退出
// EXPECT_ 断言失败则继续运行
// 断言宏必须在单元测试宏函数中使用

// 测试名称，测试用例名称
TEST(test, less_than)
{
    int age = 20;
    ASSERT_LT(age, 18);
    printf("OK!\n");
}

TEST(test, great_than)
{
    int age = 20;
    ASSERT_GT(age, 18);
    printf("OK!\n");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}