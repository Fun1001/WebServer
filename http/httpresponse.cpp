/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-04 16:21:15
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 16:37:32
 * @FilePath: /WebServer/http/httpresponse.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "httpresponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
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

HttpResponse::HttpResponse(): _code(-1),_path(""), _srcDir(""), _isKeepAlive(false), _mmFile(nullptr), _mmFileStat({.st_dev = 0})
{
}

HttpResponse::~HttpResponse()
{
    
}

void HttpResponse::MakeResponse(Buffer& buff)
{
    
}

void HttpResponse::UnmapFile()
{
    
}

char* HttpResponse::File()
{
    
}

size_t HttpResponse::FileLen() const
{
    
}

void HttpResponse::ErrorContent(Buffer& buff, std::string message)
{
    
}

void HttpResponse::_AddStateLine(Buffer& buff)
{
    
}

void HttpResponse::_AddHeader(Buffer& buff)
{
    
}

void HttpResponse::_AddContent(Buffer& buff)
{
    
}

void HttpResponse::_ErrorHtml()
{
    
}

std::string HttpResponse::_GetFileType()
{
    
}
