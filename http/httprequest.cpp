/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-27 22:17:29
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 16:07:29
 * @FilePath: /WebServer/http/httprequest.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "httprequest.h"
#include <cassert>
#include <nlohmann/json_fwd.hpp>
#include <regex>
#include <string>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "../pool/mysqlMgr.h"

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

HttpRequest::HttpRequest(): _state(PAESE_STATE::REQUEST_LINE), _method(""),_path(""), _version(""), _body(""), _header(), _post() {}

bool HttpRequest::parse(Buffer& buff)
{
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0){
        return false;
    }
    while (buff.ReadableBytes() && _state != PAESE_STATE::FINISH) {
        //找buff中未读数据中是否有终止符"\r\n",逐行读取
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);
        switch (_state) {
        case PAESE_STATE::REQUEST_LINE:
            if(!_ParseRequestLine(line)){
                return false;
            }
            _ParsePath();
            break;
        case PAESE_STATE::HEADERS:
            _ParseHeader(line);
            if(buff.ReadableBytes() <= 2){
                _state = PAESE_STATE::FINISH;
            }
            break;
        case PAESE_STATE::BODY:
            _ParseBody(line);
            break;
        default:
            break;
        }
        if (lineEnd == buff.BeginWrite()) {
            break;
        }
        // 读取解析完一行http请求，更新一下缓冲区
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", _method.c_str(), _path.c_str(), _version.c_str());
    return true;
}

std::string HttpRequest::getPath() const
{
    return _path;
}

std::string& HttpRequest::getPath()
{
    return _path;
}

std::string HttpRequest::method() const
{
    return _method;
}

std::string HttpRequest::version() const
{
    return _version;
}

std::string HttpRequest::getPost(const std::string& key) const
{
    assert(key != "");
    if(_post.count(key) == 1){
        return _post.find(key)->second;
    }
    return "";
}

std::string HttpRequest::getPost(const char* key) const
{
    assert(key != nullptr);
    if(_post.count(key) == 1){
        return _post.find(key)->second;
    }
    return "";
}

bool HttpRequest::isKeepAlive() const
{
    if(_header.count("Connection") == 1){
        return _header.find("Connection")->second == "keep-alive" && _version == "1.1";
    }
    return false;
}


// 解析HTTP请求行，并从中提取出请求方法、请求路径和HTTP版本。
bool HttpRequest::_ParseRequestLine(const std::string& line)
{
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");

    //匹配结果存储
    smatch subMatch;
    if(regex_match(line, subMatch, patten)){
        _method = subMatch[1];
        _path = subMatch[2];
        _version = subMatch[3];
        _state = PAESE_STATE::HEADERS;
        return true;
    }
    LOG_ERROR("Request Error!");
    return false;
}

void HttpRequest::_ParseHeader(const std::string& line)
{
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)){
        _header[subMatch[1]] = subMatch[2];
    }
    else {
        _state = PAESE_STATE::BODY;
    }
}

void HttpRequest::_ParseBody(const std::string& line)
{
    _body = line;
    _ParsePost();
    _state = PAESE_STATE::FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::_ParsePath()
{
    if(_path == "/"){
        _path = "/index.html";
    }
    else {
        for(auto& item : DEFAULT_HTML){
            if(item == _path){
                _path += ".html";
                break;
            }
        }
    }
}

void HttpRequest::_ParsePost()
{
    if(_method == "POST" && _header["Content-Type"] == "application/json"){
        _ParseFromJson();
        if(DEFAULT_HTML_TAG.count(_path)){
            int tag = DEFAULT_HTML_TAG.find(_path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1){
                bool isLogin = (tag == 1);
                if(UserVerify(_post["username"], _post["password"], isLogin)){
                    _path = "/welcome.html";
                }
                else {
                    _path = "/error.html";
                }
            }
        }
    }
}

//弃用
void HttpRequest::_ParseFromUrlEncoded()
{
    if(_body.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = _body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = _body[i];
        switch (ch) {
        case '=':
            key = _body.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            _body[i] = ' ';
            break;
        case '%':
            num = ConverHex(_body[i + 1]) * 16 + ConverHex(_body[i + 2]);
            _body[i + 2] = num % 10 + '0';
            _body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = _body.substr(j, i - j);
            j = i + 1;
            _post[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(_post.count(key) == 0 && j < i) {
        value = _body.substr(j, i - j);
        _post[key] = value;
    }
}

void HttpRequest::_ParseFromJson()
{
    if(_body.size() == 0){
        return;
    }
    try {
        auto json_body = nlohmann::json::parse(_body);
        for(auto& el : json_body.items()){
            _post[el.key()] = el.value();
            LOG_DEBUG("%s = %s", el.key().c_str(), el.value().dump().c_str());
        }
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Parse Error: %s", e.what());
    }
}

bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool isLogin)
{
    if(name == "" || pwd == "") {
        return false;
    }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());

    bool flag = false;
    if(!isLogin) {
        flag = true;
    }
    
    if (isLogin) {
        bool pwd_valid = MysqlMgr::GetInstance()->UserVerify(name, pwd, isLogin);
        if(!pwd_valid){
            flag = false;
            LOG_ERROR("pwd error!");
        }
        else {
            LOG_INFO("Login sucess");
        }
    }

    if (!isLogin && flag == true) {
        LOG_INFO("regirster");
        bool Reg = MysqlMgr::GetInstance()->Regirster(name, pwd);
        if(!Reg){
            flag = false;
            LOG_ERROR("failed reg");
        }
        flag = true;
    }
    LOG_DEBUG( "UserVerify success!!");
    return flag;   
}

int HttpRequest::ConverHex(char ch)
{
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}
