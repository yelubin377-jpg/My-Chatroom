MySqlClient 理解：

1. 连接对象 ， IP地址 ， 用户名 ， 密码 ， 端口 ， unix socket/nullptr , 额外选项  - mysql_real_connect

2. 复盘点：GPH/GGH两个函数的参数是否有Json::value 卡了一会儿，还有返回值类型，后面回来复盘的时候可以重点关注一下

3. 参数不全: 开始只写了一个SaveHistory , 在准备往PrivateChatHandler 和 GroupChatHandler 里面接接口的时候发现就是SaveHistory他没有groupid参数，只顾私聊了，下次写函数记得考虑全面一些;

4.