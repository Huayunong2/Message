#ifndef __M_HELPER_H__
#define __M_HELPER_H__

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <fstream>
#include "mq_logger.hpp"

namespace MQ
{
    class SqliteHelper
    {
    public:
        typedef int (*SqliteCallbaack)(void *, int, char **, char **);

        SqliteHelper(const std::string &dbfile)
            : _dbfile(dbfile),
              _handler(nullptr)
        {
        }

        bool open(int safe_leve = SQLITE_OPEN_FULLMUTEX)
        {
            int ret = sqlite3_open_v2(_dbfile.c_str(), &_handler, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | safe_leve, nullptr);
            if (ret != SQLITE_OK)
            {
                ELOG("创建/打开数据库失败: %s", sqlite3_errmsg(_handler));
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
                ELOG("[%s]执行语句失败: %s", sql.c_str(), sqlite3_errmsg(_handler));
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

    class StrHelper
    {
    public:
        static size_t split(const std::string &str, const std::string &sep, std::vector<std::string> &result)
        {
            size_t pos, idx = 0;
            while (idx < str.size())
            {
                pos = str.find(sep, idx);
                if (pos == std::string::npos)
                {
                    result.push_back(str.substr(idx));
                    return result.size();
                }
                if (pos == idx)
                {
                    idx = pos + sep.size();
                    continue;
                }
                result.push_back(str.substr(idx, pos - idx));
                idx = pos + sep.size();
            }
            return result.size();
        }
    };

    class UUIDHelper
    {
    public:
        static std::string uuid()
        {
            std::random_device rd;
            std::mt19937_64 gernator(rd());
            std::uniform_int_distribution<int> distribution(0, 255);
            std::stringstream ss;
            for (int i = 0; i < 8; i++)
            {
                ss << std::setw(2) << std::setfill('0') << std::hex << distribution(gernator);
                if (i == 3 || i == 5 || i == 7)
                {
                    ss << "-";
                }
            }
            static std::atomic<size_t> seq(1);
            size_t num = seq.fetch_add(1);
            for (int i = 7; i >= 0; i--)
            {
                ss << std::setw(2) << std::setfill('0') << std::hex << ((num >> i * 8) & 0xff);
                if (i == 6)
                    ss << "-";
            }
            return ss.str();
        }
    };

    class FileHelper
    {
    public:
        FileHelper(const std::string &filename)
            : _filename(filename)
        {
        }

        // 保证跨平台可移植性
        bool exists()
        {
            struct stat st;
            return (stat(_filename.c_str(), &st) == 0);
        }

        size_t size()
        {
            struct stat st;
            int ret = stat(_filename.c_str(), &st);
            if (ret < 0)
            {
                return 0;
            }
            else
                return st.st_size;
        }

        bool read(std::string &body)
        {
            // 获取文件大小，根据文件大小调整body空间
            size_t fsize = this->size();
            body.resize(fsize); // 调整body空间防止被认定为非法访问
            return read(&body[0], 0, fsize);
        }

        // 读指定区域
        bool read(char *body, size_t offset, size_t len)
        {
            // 1.打开文件
            std::ifstream ifs(_filename, std::ios::binary | std::ios::in);
            if (ifs.is_open() == false)
            {
                ELOG("%s 文件打开失败!", _filename.c_str());
                return false;
            }
            // 2.跳转文件读写位置
            ifs.seekg(offset, std::ios::beg); // 起始位置跳转到offset
            // 3.读取文件数据
            ifs.read(body, len);
            if (ifs.good() == false)
            {
                ELOG("%s 文件读取数据失败!", _filename.c_str());
                ifs.close();
                return false;
            }
            // 4.关闭文件
            ifs.close();
            return true;
        }

        bool write(const std::string &body)
        {
            return write(body.c_str(), 0, body.size());
        }

        // 写入到指定位置
        bool write(const char *body, size_t offset, size_t len)
        {
            // 1.打开文件，不能用ofstream，跳转指定位置需要读权限，ofstream没有读权限
            std::fstream fs(_filename, std::ios::binary | std::ios::in | std::ios::out);
            if (fs.is_open() == false)
            {
                ELOG("%s 文件打开失败!", _filename.c_str());
                return false;
            }
            // 2.跳转文件读写位置
            fs.seekp(offset, std::ios::beg); // 起始位置跳转到offset
            // 3.写入数据
            fs.write(body, len);
            if (fs.good() == false)
            {
                ELOG("%s 文件写入数据失败!", _filename.c_str());
                fs.close();
                return false;
            }
            // 4.关闭文件
            fs.close();
            return true;
        }

        bool rename(const std::string &nname)
        {
            // 用的全局rename，不是自己的
            return (::rename(_filename.c_str(), nname.c_str()) == 0);
        }

        static bool createFile(const std::string &filename)
        {
            std::fstream ofs(filename, std::ios::binary | std::ios::out);
            if (ofs.is_open() == false)
            {
                ELOG("%s 文件打开失败!", filename.c_str());
                return false;
            }
            ofs.close();
            return true;
        }

        static bool removeFile(const std::string &filename)
        {
            return (::remove(filename.c_str()) == 0);
        }

        static bool createDirectory(const std::string &path)
        {
            // /aaa/bbb/ccc多级路径创建中，我们需要从第一个父级目录开始创建
            size_t pos, idx = 0;
            while (idx < path.size())
            {
                pos = path.find("/", idx);
                if (pos == std::string::npos)
                {
                    return (mkdir(path.c_str(), 0775) == 0);
                }
                std::string subpath = path.substr(0, pos);
                int ret = mkdir(subpath.c_str(), 0775);
                if (ret != 0 && errno != EEXIST)
                {
                    ELOG("创建目录 %s 失败: %s", subpath.c_str(), strerror(errno));
                    return false;
                }
                idx = pos + 1;
            }
            return true;
        }

        static bool removeDirectory(const std::string &path)
        {
            std::string cmd = "rm -rf " + path;
            return (system(cmd.c_str()) != -1);
        }

        // 获取父级目录
        static std::string parentDirectory(const std::string &path)
        {
            size_t pos = path.find_last_of("/\\");
            if (pos == std::string::npos)
            {
                return ".";
            }
            return path.substr(0, pos);
        }

    private:
        std::string _filename;
    };
}
#endif