#include <iostream>
#include <thread>
#include <future>
#include <memory>

// 是一个模板类，实例化的对象可以对一个函数进行二次封装
// package可以通过get_future获取一个future对象，来获取这个封装函数的结果
int Add(int num1, int num2)
{
    return num1 + num2;
}

int main()
{
    // 二次封装，但是不能将其当成一个普通的函数对待
    // std::packaged_task<int(int, int)> task(Add);
    // std::future<int> fu = task.get_future();

    // 可以把task定义为一个指针，传到线程中，然后解引用
    // 但是如果单纯指针指向一个对象，存在生命周期的问题，可能出现风险
    // 所以要在堆上new，用智能指针管理它的生命周期
    auto ptask = std::make_shared<std::packaged_task<int(int, int)>>(Add);
    std::future<int> fu = ptask->get_future();
    std::thread thr([ptask]()
                    { (*ptask)(11, 22); });
    int sum = fu.get();
    std::cout << sum << std::endl;
    thr.join();
    return 0;
}