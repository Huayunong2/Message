#include <iostream>
#include <string>
#include <vector>

size_t split(const std::string &str, const std::string &sep, std::vector<std::string> &result)
{
    // news...music.#.pop
    // 从0开始查找指定字符的位置，找到后分割
    // 从上次查找的位置继续向后查找指定字符
    size_t pos, idx = 0;
    while (idx < str.size())
    {
        pos = str.find(sep, idx);
        if (pos == std::string::npos)
        {
            // 没找到，则从查找位置截取到末尾
            result.push_back(str.substr(idx));
            return result.size();
        }
        // 两个分隔符之间没有数据，或者说查找起始位置就是分隔符
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

int main()
{
    std::string str = "...news....music.##.pop...";
    std::vector<std::string> arry;
    int n = split(str, ".", arry);
    for (auto &s : arry)
    {
        std::cout << s << std::endl;
    }
    return 0;
}