#include <iostream>
#include <thread>
#include <future>
#include <unistd.h>

int Add(int num1, int num2)
{
    std::cout << "加法\n";
    return num1 + num2;
}

int main()
{
    // std::async()(func,...)    std::async(policy,func,...);
    std::cout << "----------1-----------" << std::endl;
    std::future<int> result = std::async(std::launch::deferred, Add, 11, 22); // deferred策略下，异步任务不会立即启动，而是在调用get()或wait()时执行。
    std::cout << "----------2-----------" << std::endl;
    int sum = result.get(); // 调用get以后才执行异步
    std::cout << "----------3-----------" << std::endl;
    std::cout << sum << std::endl;
    return 0;
}