#ifndef __M_ROUTE_H__
#define __M_ROUTE_H__
#include <iostream>
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_msg.pb.h"

namespace MQ
{
    class Router
    {
    public:
        Router()
        {
        }

        static bool isLegalRoutingKey(const std::string &routing_key)
        {
            for (auto &c : routing_key)
            {
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    (c == '_' || c == '.'))
                {
                    continue;
                }
                return false;
            }
            return true;
        }
        static bool isLegalBindingKey(const std::string &binding_key)
        {
            // 1.判断是否包含有非法字符
            for (auto &c : binding_key)
            {
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    (c == '_' || c == '.') ||
                    (c == '*' || c == '#'))
                {
                    continue;
                }
                return false;
            }
            // 2.*和#只能独立存在
            std::vector<std::string> sub_words;
            StrHelper::split(binding_key, ".", sub_words);
            for (auto &word : sub_words)
            {
                if (word.size() > 1 &&
                    (word.find("*") != std::string::npos ||
                     word.find("#") != std::string::npos))
                {
                    return false;
                }
            }
            // 3.*和#不能连续出现
            for (int i = 1; i < sub_words.size(); i++)
            {
                if (sub_words[i] == "#" && sub_words[i - 1] == "*")
                {
                    return false;
                }
                if (sub_words[i] == "#" && sub_words[i - 1] == "#")
                {
                    return false;
                }
                if (sub_words[i] == "*" && sub_words[i - 1] == "#")
                {
                    return false;
                }
            }
            return true;
        }
        static bool route(ExchangeType type, const std::string &routing_key, const std::string &binding_key)
        {
            if (type == ExchangeType::DIRECT)
            {
                return (routing_key == binding_key);
            }
            else if (type == ExchangeType::FANOUT)
            {
                return true;
            }
            // 主题交换：要进行模式匹配 news.# & news.music.pop
            // 1.将binding_key与routing_key进行字符串分割，得到各个的单词数组
            std::vector<std::string> bkeysy, rkeys;
            int n_bkey = StrHelper::split(binding_key, ".", bkeysy);
            int n_rkey = StrHelper::split(routing_key, ".", rkeys);
            // 2.定义标记数组，并初始化[0][0]位置为true，其他位置为false
            std::vector<std::vector<bool>> dp(n_bkey + 1, std::vector<bool>(n_rkey + 1, false));
            dp[0][0] = true;
            // 3.如果binding_key以#起始，则将#对应行的第0列置为1
            for (int i = 1; i <= n_bkey; i++)
            {
                if (bkeysy[i - 1] == "#")
                {
                    dp[i][0] = true;
                    continue;
                }
                break;
            }
            // 4.使用routing_key的单词数组与binding_key的单词数组进行匹配，并标记数组
            for (int i = 1; i <= n_bkey; i++)
            {
                for (int j = 1; j <= n_rkey; j++)
                {
                    // 如果binding_key是两个单词相同，或者是*，表示单词匹配成功，从左上方继承
                    if (bkeysy[i - 1] == rkeys[j - 1] || bkeysy[i - 1] == "*")
                    {
                        dp[i][j] = dp[i - 1][j - 1];
                    }
                    // 如果binding_key是#，则从左上，左边，上边继承结果
                    else if (bkeysy[i - 1] == "#")
                    {
                        dp[i][j] = dp[i - 1][j - 1] | dp[i][j - 1] | dp[i - 1][j];
                    }
                }
            }
            return dp[n_bkey][n_rkey];
        }

    private:
        std::unordered_map<std::string, std::string> routingTable;
    };
}

#endif