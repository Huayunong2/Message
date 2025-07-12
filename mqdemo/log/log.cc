#include <iostream>
#include <ctime>

// 封装一个日志宏，通过日志宏进行日志的打印，在打印的信息前带有系统时间以及文件名和行号
// [time][file:line] 打开文件失败

#define DBG_LEVEL 0
#define INF_LEVEL 1
#define ERR_LEVEL 2
#define DEFAULT_LEVEL DBG_LEVEL
#define LOG(level_str, level, format, ...)                                                                   \
    {                                                                                                        \
        if (level >= DEFAULT_LEVEL)                                                                          \
        {                                                                                                    \
            time_t t = time(nullptr);                                                                        \
            struct tm *ptm = localtime(&t);                                                                  \
            char time_str[32];                                                                               \
            strftime(time_str, 31, "%H:%M:%S", ptm);                                                         \
            printf("[%s][%s][%s:%d]\t" format "\n", level_str, time_str, __FILE__, __LINE__, ##__VA_ARGS__); \
        }                                                                                                    \
    }
#define DLOG(format, ...) LOG("DEBUG", DBG_LEVEL, format, ##__VA_ARGS__)
#define ILOG(format, ...) LOG("INFO", INF_LEVEL, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG("ERR", ERR_LEVEL, format, ##__VA_ARGS__)
int main()
{
    ILOG("HEllo");
    return 0;
}