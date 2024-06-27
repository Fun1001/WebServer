<!--
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-24 20:38:36
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-27 21:42:47
 * @FilePath: /WebServer/pool/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# 数据库连接池
    使用单例模式和队列来实现数据库连接池，同时利用RAII机制来释放数据连接。
    数据库访问的一般流程：
    1. 当系统需要访问数据库时，先系统创建数据库连接
    2. 接着完成数据库操作
    3. 最后断开数据库连接

    据库的资源我们使用信号量进行同步，所以，将信号量初始化为数据库的连接总数；

    初始化的代码十分简单，只有两个不常见的API需要了解
    mysql_init
    mysql_real_connection

## 数据库访问函数
1. 获取数据库连接
2. 释放数据库连接
3. 销毁数据库连接

    除此之外，我们再设计一个共有函数来获得私有变量空余的连接数量，就完成了数据库访问函数的设计；注意，无论是获取，释放还是销毁，我们都要用mutex来保证线程同步；同时，获取连接前需要wait()阻塞等到临界区有资源，释放连接后需要post()来提醒其他线程临界区有新资源

# 半同步/半反应堆线程池
从理论上来讲，Proactor的效率是要比Reactor要高的，但是因为Linux对异步IO的系统调用远不如同步IO完备，所以我们还是选择了Reactor，但同时也在应用层面实现了模拟Proactor的功能，即把内核应该做的事情交给主线程来完成，这样肯定没有真正的Proactor快，但帮助我们理解Proactor事件处理模式已经够了。

其实这里讲到的主线程，自然就有工作线程，这就涉及到我们今天的知识点了，半同步/半反应堆的并发模式。并发中的半同步大家理解，即代码按书写顺序进行嘛，那半反应堆是什么？在我们上一章的双向链表设计的信号机制那里其实我们就讲了同步和异步，而这里的反应堆其实就是异步的一种，所谓反应堆就是reactive，它通过epoll机制将接收到sockfd（包括IO事件和信号）中的读写事件拿出来，就完成了所谓的异步。
![半同步/半反应堆线程池](https://img-blog.csdnimg.cn/img_convert/76cabfef907ee4282d4335c042ef022c.png)