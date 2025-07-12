#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

class SqliteHelper
{
public:
    // 修正：将vod改为void
    typedef int (*SqliteCallbaack)(void *, int, char **, char **);

    SqliteHelper(const std::string &dbfile)
        : _dbfile(dbfile),
          _handler(nullptr)
    {
    }

    bool open(int safe_leve = SQLITE_OPEN_FULLMUTEX)
    {
        // 修正：使用_handler而不是handler
        int ret = sqlite3_open_v2(_dbfile.c_str(), &_handler, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | safe_leve, nullptr);
        if (ret != SQLITE_OK)
        {
            std::cout << "打开数据库失败:";
            std::cout << sqlite3_errmsg(_handler) << std::endl;
            return false;
        }
        return true;
    }

    bool exec(const std::string &sql, SqliteCallbaack cb, void *arg)
    {
        int ret = sqlite3_exec(_handler, sql.c_str(), cb, arg, nullptr);
        if (ret != SQLITE_OK)
        {
            std::cout << sql << std::endl;
            std::cout << "执行语句失败:";
            std::cout << sqlite3_errmsg(_handler) << std::endl;
            return false;
        }
        return true;
    }

    void close()
    {
        if (_handler)
            sqlite3_close_v2(_handler);
    }

private:
    std::string _dbfile;
    sqlite3 *_handler;
};