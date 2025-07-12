#include <iostream>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <atomic>

int main()
{
    for (int n = 0; n <= 100; n++)
    {
        std::random_device rd;
        // size_t num = rd(); // 机器随机数，效率较低
        // 通过机器随机数作为种子，用mt19937生成随机数
        std::mt19937_64 gernator(rd()); // 通过梅森旋转算法，生成一个伪随机数
        // 限定数字区间(0-255)
        std::uniform_int_distribution<int> distribution(0, 255);
        // 将生成的数字转换为16进制数字字符
        std::stringstream ss;
        for (int i = 0; i < 8; i++)
        {
            // 宽度不为2，则在前面加一个0
            ss << std::setw(2) << std::setfill('0') << std::hex << distribution(gernator);
            if (i == 3 || i == 5 || i == 7)
            {
                ss << "-";
            }
        }
        static std::atomic<size_t> seq(1); // 定义一个原子类型整数，初始化为1
        size_t num = seq.fetch_add(1);     // 8字节
        for (int i = 7; i >= 0; i--)
        {
            ss << std::setw(2) << std::setfill('0') << std::hex << ((num >> i * 8) & 0xff);
            if (i == 6)
                ss << "-";
        }
        std::cout << ss.str() << std::endl;
    }
    return 0;
}