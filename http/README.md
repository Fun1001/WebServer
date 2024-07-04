<!--
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-28 14:54:54
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-28 15:58:48
 * @FilePath: /WebServer/http/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
一个完整的HTTP请求由多个部分组成，包括请求行、请求头和请求体（可选）。以下是一个典型的HTTP请求的示例：
请求行 (Request Line)
    请求行包含请求方法、请求路径和HTTP版本。示例：

    GET /index.html HTTP/1.1

请求头 (Request Headers)
    请求头包含多个键值对，每个键值对表示一个HTTP头字段。示例：

    Host: www.example.com
    User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3
    Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
    Accept-Language: en-US,en;q=0.5
    Accept-Encoding: gzip, deflate, br
    Connection: keep-alive
    Upgrade-Insecure-Requests: 1

空行
    请求头和请求体之间有一个空行，用于分隔头部和主体。如果没有请求体，则请求头后直接是空行。
请求体 (Request Body)
    请求体是可选的，通常用于POST、PUT等方法来发送数据。示例：
    
    name=John&age=30