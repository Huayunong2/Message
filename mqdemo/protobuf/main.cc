#include "contacts.pb.h"
#include <iostream>

int main()
{
    contacts::contact conn;

    conn.set_sn(10001);
    conn.set_name("小明");
    conn.set_score(60.5);

    // 持久化数据放在str对象中，可以对str进行持久化或网络传输
    std::string str = conn.SerializeAsString();

    // 反序列化
    contacts::contact stu;
    bool ret = stu.ParseFromString(str);
    if (!ret)
    {
        std::cout << "反序列化失败!\n";
        return -1;
    }
    std::cout << stu.sn() << std::endl;
    std::cout << stu.name() << std::endl;
    std::cout << stu.score() << std::endl;
    
    return 0;
}