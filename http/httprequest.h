/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-27 22:17:19
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 14:28:23
 * @FilePath: /WebServer/http/httprequest.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql_driver.h>
#include "../buffer/buffer.h"
#include "../log/log.h"
//#include "../pool/sqlconnpool.h"

class HttpRequest{
public:
    enum PAESE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTED,
    };

    HttpRequest();

    ~HttpRequest() = default;

    bool parse(Buffer& buff);

    std::string getPath() const;

    std::string& getPath();

    std::string method() const;

    std::string version() const;

    std::string getPost(const std::string& key) const;

    std::string getPost(const char* key) const;

    bool isKeepAlive() const;

private:
    bool _ParseRequestLine(const std::string& line);
    void _ParseHeader(const std::string& line);
    void _ParseBody(const std::string& line);
    void _ParsePath();
    void _ParsePost();
    void _ParseFromUrlEncoded();
    void _ParseFromJson();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PAESE_STATE _state;
    std::string _method, _path, _version, _body;
    std::unordered_map<std::string, std::string> _header;
    std::unordered_map<std::string, std::string> _post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

};


#endif // __HTTPREQUEST_H__