/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-04 16:21:15
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-05 16:07:46
 * @FilePath: /WebServer/http/httpresponse.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "httpresponse.h"
#include <cassert>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".json", "text/json"},
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse(): _code(-1), _isKeepAlive(false), _path(""), _srcDir(""), _mmFile(nullptr), _mmFileStat({.st_dev = 0})
{
}

HttpResponse::~HttpResponse()
{
    
}

void HttpResponse::Init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code)
{
    assert(srcDir != "");
    if (_mmFile) {
        UnmapFile();
    }
    _code = code;
    _isKeepAlive = isKeepAlive;
    _path = path;
    _srcDir = srcDir;
    _mmFile = nullptr;
    _mmFileStat = {.st_dev = 0};
}

void HttpResponse::MakeResponse(Buffer& buff)
{
    /* 判断请求的资源文件 */
    // stat 是一个 POSIX 函数，用于获取文件的状态信息，并将结果存储在传入的 struct stat 结构体 _mmFileStat 中。
    // S_ISDIR 是一个宏，用于检查给定的文件模式是否表示一个目录。
    if(stat((_srcDir+_path).data(), &_mmFileStat) < 0 || S_ISDIR(_mmFileStat.st_mode)){
        _code = 404;
    }
    // 是否可读
    else if(!(_mmFileStat.st_mode & S_IROTH)){
        _code = 403;
    }
    else if (_code == -1) {
        _code = 200;
    }

    _ErrorHtml();
    _AddStateLine(buff);
    _AddHeader(buff);
    _AddContent(buff);
}

// 释放通过mmap系统调用映射到进程地址空间的文件
void HttpResponse::UnmapFile()
{
    if(_mmFile){
        munmap(_mmFile, _mmFileStat.st_size);
        _mmFile = nullptr;
    }
}

char* HttpResponse::File()
{
    return _mmFile;
}

size_t HttpResponse::FileLen() const
{
    return _mmFileStat.st_size;
}

void HttpResponse::ErrorContent(Buffer& buff, std::string message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";

    if (HttpResponse::CODE_STATUS.count(_code) == 1) {
        status = HttpResponse::CODE_STATUS.find(_code)->second;
    }
    else {
        status = "Bad Request";
    }

    body += to_string(_code) + ":" + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>FunWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

void HttpResponse::_AddStateLine(Buffer& buff)
{
    std::string status;
    if(HttpResponse::CODE_STATUS.count(_code) == 1){
        status = HttpResponse::CODE_STATUS.find(_code)->second;
    }
    else {
        _code = 400;
        status = HttpResponse::CODE_STATUS.find(_code)->second;
    }
    buff.Append("HTTP/1.1" + to_string(_code) + " " + status + "\r\n");
}

void HttpResponse::_AddHeader(Buffer& buff)
{
    buff.Append("Connection: ");
    if(_isKeepAlive){
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + _GetFileType() + "\r\n");
}

void HttpResponse::_AddContent(Buffer& buff)
{
    int srcFd = open((_srcDir + _path).data(), O_RDONLY);
    if(srcFd < 0){
        ErrorContent(buff, "File NotFound!");
        return;
    }
    /* 将文件映射到内存提高文件的访问速度 
    MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s", (_srcDir + _path).data());
    int* mmRet = (int*)mmap(0, _mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1){
        ErrorContent(buff, "File NotFound!");
        return;
    }
    _mmFile = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + to_string(_mmFileStat.st_size) + "\r\n\r\n");
}

void HttpResponse::_ErrorHtml()
{
    if(HttpResponse::CODE_PATH.count(_code) == 1){
        _path = HttpResponse::CODE_PATH.find(_code)->second;
        stat((_srcDir + _path).data(), &_mmFileStat);
    }
}

std::string HttpResponse::_GetFileType()
{
    std::string::size_type idx = _path.find_last_of('.');
    if(idx == std::string::npos){
        return "text/plain";
    }
    std::string suffix = _path.substr(idx);
    if(HttpResponse::SUFFIX_TYPE.count(suffix) == 1){
        return HttpResponse::SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}
