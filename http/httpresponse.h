/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-04 16:21:10
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 16:38:54
 * @FilePath: /WebServer/http/httpresponse.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__
#include <cstddef>
#include <string>
#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const {return _code;};

private:
    void _AddStateLine(Buffer& buff);
    void _AddHeader(Buffer& buff);
    void _AddContent(Buffer& buff);

    void _ErrorHtml();
    std::string _GetFileType();

    int _code;
    bool _isKeepAlive;
    std::string _path;
    std::string _srcDir;

    // 使用 _mmFile 指针存储文件内容或路径，并使用 _mmFileStat 结构体存储文件的状态信息。
    char* _mmFile;
    //  POSIX 标准中定义的用于描述文件状态的结构体,获取文件的属性和状态信息。
    struct stat _mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;

};

#endif // __HTTPRESPONSE_H__