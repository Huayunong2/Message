#include "sqlite.hpp"
#include <cassert>

int select_stu_callback(void *arg, int col_count, char **result, char **fileds_name)
{
    std::vector<std::string> *arry = (std::vector<std::string> *)arg;
    arry->push_back(result[0]);
    return 0; // 回调函数成功必须返回0，否则认为异常
}

int main()
{
    SqliteHelper helper("./test.db");
    // 1.创建/打开库文件
    assert(helper.open());
    // 2.创建表(不存在则创建库)
    const char *ct = "create table if not exists student(sn int primary key, name varchar(32),age int);";
    assert(helper.exec(ct, nullptr, nullptr));
    // 3.增删查改
    // const char* insert_sql = "insert into student values(1,'小明',18),(2,'小黑',19),(3,'小红',18);";
    // assert(helper.exec(insert_sql,nullptr,nullptr));

    // const char* update_sql = "update student set name = '芜湖' where sn = 1;";
    // assert(helper.exec(update_sql,nullptr,nullptr));

    // const char *delete_sql = "delete from student where sn = 3;";
    // assert(helper.exec(delete_sql,nullptr,nullptr));

    const char *select_sql = "select name from student;";
    std::vector<std::string> arry;
    assert(helper.exec(select_sql, select_stu_callback, &arry));
    for (auto &name : arry)
    {
        std::cout << name << std::endl;
    }

    // 4.关闭
    helper.close();
    return 0;
}