#include "../mqcommon/mq_helper.hpp"

int main()
{
    // MQ::FileHelper helper("../mqcommon/logger.hpp");
    // DLOG("是否存在: %d", helper.exists());
    // DLOG("文件大小: %ld", helper.size());

    // MQ::FileHelper tmp_helper("./aaa/bbb/ccc/tmp.hpp");
    // if(tmp_helper.exists() == false)
    // {
    //     std::string path = MQ::FileHelper::parentDirectory("./aaa/bbb/ccc/tmp.hpp");
    //     if(MQ::FileHelper(path).exists() == false){
    //         MQ::FileHelper::createDirectory(path);
    //     }
    //     MQ::FileHelper::createFile("./aaa/bbb/ccc/tmp.hpp");
    // }

    // std::string body;
    // helper.read(body);
    // DLOG("%s", body.c_str());
    // tmp_helper.write(body);

    // MQ::FileHelper tmp_helper("./aaa/bbb/ccc/tmp.hpp");
    // char str[16] = {0};
    // tmp_helper.read(str, 8, 11);
    // DLOG("[%s]", str);
    // tmp_helper.write("123456789", 8, 11);

    // tmp_helper.rename("./aaa/bbb/ccc/test.hpp");

    // MQ::FileHelper::removeFile("./aaa/bbb/ccc/test.hpp");

    // MQ::FileHelper::removeDirectory("./aaa");

    return 0;
}