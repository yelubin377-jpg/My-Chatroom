连接数据库 -> 发SQL 然后 拿结果
#include <mysql/mysql.h>
#include <cstdio>

int main()
{
    MYSQL* conn = mysql_init(nullptr);

    if(!mysql_real_connect(conn, "127.0.0.1", "root", "123456",
                           "chatroom", 3306, nullptr, 0))
    {
        printf("连不上: %s\n", mysql_error(conn));
        return -1;
    }
    
    mysql_query(conn, "SELECT id, content FROM messages LIMIT 5");
    
    MYSQL_RES* result = mysql_store_result(conn);
}

mysql_init(nullptr) 分配Mysql对象，nullptr默认设置
！！！mysql_real_connect > 真正连接，！！！参数是：(连接对象 ， IP地址 ， 用户名 ， 密码 ， 数据库，端口 ， unix socket/nullptr , 额外选项)
mysql_query > 往Mysql丢一条SQL字符串，甑山茶改

musql_store_result 把查询结果全部把得取到本地内存\

```c++


int numFields = mysql_num_fields(result);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(result)))
    {
        for(int i = 0; i < numFields; i++)
        {
            printf("%s ", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

mysql_free_result(result);
mysql_close(conn);



```

explain:
    mysql_num_fields --- 查询结果有多少列 
    mysql_fetch_row --- 每次调用返回下一行，返回nullptr 说明读完了

  - row[i] — 第 i 列的值，永远是 char* 类型，不管数据库里是 int 还是 varchar，拿回来都是字符串。如果这列是 NULL，row[i] 就是 nullptr
   mysql_free_result — 释放结果集，不调会内存泄漏
    mysql_close 断连



每个MYSQL* 句柄都对应一个独立的数据库连接，不能在多线程间共享（每个线程创建独立的句柄
生命周期
从 init 到 close


mysql_init()

**MYSQL *mysql_init(MYSQL *mysql)**


初始化一个MYSQL对象，用于后续数据库连接操作

参数

mysql：指向MYSQL结构体的指针。

如果传入NULL，系统会自动分配并返回一个新对象。

返回值

成功：返回初始化后的MYSQL * 句柄

失败：NULL（如内存不足）
————————————————



**MYSQL *mysql_real_connect(MYSQL *mysql, 
const char *host, 
const char *user, 
const char *passwd, 
const char *db,
 unsigned int port, 
 const char *unix_socket, 
 unsigned long client_flag)

mysql：有mysql_init初始化的MYSQL对象

host：主机名或者IP地址，如果是NULL，”localhost“，”127.0.0.1”被视为与本地主机连接

user：登陆用户名

passwd：登陆密码

db：默认连接的数据库名（可选）

port：服务器端口号（0表示默认端口3306）

unix_socket：套接字或命名管道（通常为NULL）

client_flag：连接标志

成功：返回传入的MYSQL *句柄

失败：NULL，可通过mysql_error(mysql)获取错误信息。

mysql_real_query()
成功：0
失败：返回非0值，需要用mysql_error(mysql)检查错误

mysql_num_fields()
result：mysql_store_result 或者 mysql_use_result 返回的结果集

mysql_num_rows()
获取结果集中的行数（仅对 mysql_store_result 有效）
参数
result：my_result_result 返回的结果集


mysql_fetch_field()
成功：返回MYSQL_FIELD结构体指针
失败：无更多列：NULL



MYSQL_FIELD结构体
typedef struct st_mysql_field
{
  char *name;// 列名（如 "id"）
  char *org_name;// 原始列名（若使用别名时）
  char *table;// 所属表名
  char *org_table;// 原始表名（若使用别名时）
  char *db;// 所属数据库名
  char *catalog;// 目录名（通常为空）
  char *def;// 列的默认值
  unsigned long length;// 列的定义长度（如 VARCHAR(255) → 255）
  unsigned long max_length;// 结果集中实际最大长度（需调用 mysql_store_result 后有效）
  unsigned int name_length;
  unsigned int org_name_length;
  unsigned int table_length;
  unsigned int org_table_length;
  unsigned int db_length;
  unsigned int catalog_length;
  unsigned int def_length;
  unsigned int flags;// 列的标志（如 NOT_NULL_FLAG）
  unsigned int decimals;// 小数位数（如 DECIMAL(10,2) → 2）
  unsigned int charsetnr;// 字符集编号
  enum enum_field_types type;// 列的数据类型（如 MYSQL_TYPE_INT、MYSQL_TYPE_STRING）
} MYSQL_FIELD;

mysql_fetch_row()、
typedef char **MYSQL_ROW; *// 例如：row[0] 是第一列的值*



mysql_free_result()  释放mysql_store_result 分配的 结果集的内存

mysql_close()  mysql：已连接的MYSQL
关闭数据库连接，并释放MYSQL对象内存   

不能重复关闭句柄！！！--》保险一点：
if(mysql != NULL )
{
	mysql_close(mysql);
	mysql = NULL;
}



自动清理未释放的结果集：
如果忘记调用mysql_free_result()，mysql_close()会隐式释放结果集，但是显示释放更加直观，更加安全。



int mysql_set_character_set(MYSQL *mysql, const char *csname); 
设置 MySQL 客户端连接（MYSQL* 句柄）的字符集，确保客户端与服务器之间的数据传输使用指定编码。
mysql：已成功连接的MYSQL *句柄

csname：字符集名称字符串

"utf8mb4"：支持 Unicode 4 字节字符（如 Emoji）。
"gbk"：中文编码。
"binary"：二进制数据。

返回值

成功：0

失败：非0






大致基本框架：

```c++
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    //初始化
    MYSQL * conn = mysql_init(NULL);//错误1 这里参数应是NULL
    if(conn == NULL)
    {
        perror("mysql_init");
        exit(1);
    }

//连接数据库
if(!mysql_real_connect(conn,NULL,"root","","person",3306,NULL,0))
{
    fprintf(stderr,"%s\\n",mysql_error(conn));
    if(conn != NULL)
    {
        mysql_close(conn);
        conn = NULL;
    }
    exit(1);
}

mysql_set_character_set(conn, "utf8mb4");

//请求
char *arr_query1 = "select num,name from new_stu";
char *arr_query2 = "insert into new_stu values(109,'小明',18)";
unsigned len_query1 = strlen(arr_query1);
unsigned len_query2 = strlen(arr_query2);
if(mysql_real_query(conn,arr_query1,len_query1))
{
    fprintf(stderr,"%s\\n",mysql_error(conn));
    if(conn != NULL)
    {
        mysql_close(conn);
        conn = NULL;
    }
    exit(1);
}

//获取数据集
MYSQL_RES * result = NULL; 
if(!(result = mysql_store_result(conn)))
{
    fprintf(stderr,"%s\\n",mysql_error(conn));
    if(conn != NULL)
    {
        mysql_close(conn);
        conn = NULL;
    }
    exit(1);
}

//遍历数据库行数 列数
unsigned int len_column = mysql_num_fields(result);
my_ulonglong len_row = mysql_num_rows(result);
printf("column == %u,row == %llu\\n",len_column,len_row);

//遍历数据库的数据
if(len_row != 0)
{
    //遍历列的属性
    MYSQL_FIELD *filed = NULL;
    while(filed = mysql_fetch_field(result))
    {
        printf("%-10s",filed->name);
    }
    printf("\\n");

​    //遍历行数据
​    MYSQL_ROW row = NULL;
​    while(row = mysql_fetch_row(result))
​    {
​        for (size_t i = 0; i < len_column; i++)
​        {
​            printf("%-10s",row[i]);
​        }
​        printf("\\n");
​    }
}
else
{
​    //无数据
​    printf("No data on database!\\n");
}

//注意这里 这里我们在这里插入 而不是紧跟着mysql_real_query(conn,arr_query1,len_query1)插入
//让大家更好地理解 结果集的存在 和 mysql_store_result 的逻辑
if(mysql_real_query(conn,arr_query2,len_query2))
{
    fprintf(stderr,"%s\\n",mysql_error(conn));
    if(conn != NULL)
    {
        mysql_close(conn);
        conn = NULL;
    }
    exit(1);
}

//关闭数据集
mysql_free_result(result);

//关闭句柄
if(conn != NULL)
{
    mysql_close(conn);
    conn = NULL;
}

return 0;

}


```

代码犯错：
1、mysql_init 参数应是NULL，写成 还未初始化的conn

2、没有加字符格式矫正，导致数据库中文乱码

mysql_set_character_set

3、对结果集的理解有误 导致mysql_store_result

以及后续result的使用上 与理想情况出现偏差。复习时重点回想
————————————————
本md参考：
版权声明：本文为CSDN博主「长流小哥」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/2401_87244387/article/details/147724362



-- ======== 1. 建库 ========
-- 整个 MySQL 上一个"仓库"，专放这个项目的表
CREATE DATABASE mydb;
-- 进去
USE mydb;

-- ======== 2. 建表 ========
-- 在仓库里放一个"表格"，定义有哪些列
CREATE TABLE student (              -- student 是表名
    id       INT,                   -- 学号，整数
    name     VARCHAR(20),           -- 名字，最多20个字符的字符串
    age      INT                    -- 年龄，整数
);

逐行解释：

---
CREATE TABLE IF NOT EXISTS chat_history (

CREATE TABLE = 建一张新的表格。IF NOT EXISTS = 如果这张表已经存在，别报错，直接跳过。防止程序每次启动都建表报重复。chat_history = 表名。

    id INT AUTO_INCREMENT PRIMARY KEY,

id = 这一列的名字，自增序号——第一条插入自动给 1，第二条给 2，依此类推。PRIMARY KEY = 主键，保证每行唯一，通过 id 可以最快速度定位到某一行。这列跟聊天内容没关系，是给数据库自己用的。

    from_user VARCHAR(64),

from_user = 发消息的人的用户名。VARCHAR(64) = 变长字符串，最长 64 个字符。varchar 比 char 省空间——存 "alice" 只占 5 个字符，不会给你塞空格到 64。


to_user = 私聊接收方的用户名。to_group = 群聊的群 ID。对于每一条消息，这两个列只有一个有值——私聊时 to_user 有值 to_group 为 NULL，群聊时反过来。

    content TEXT,

content = 消息内容。TEXT = 长文本，不限 64 字符，可以存几千字的 JSON 字符串。

    created_at DATETIME DEFAULT CURRENT_TIMESTAMP

created_at = 这条消息什么时候发的。DATETIME = 日期+时间格式（2026-08-10 14:23:05）。DEFAULT CURRENT_TIMESTAMP = 插入时如果不指定时间，自动用当前时间填上。

-- ======== 3. 插入 ========
-- 往表格里塞一行数据
INSERT INTO student (id, name, age)
VALUES (1, '张三', 20);             -- 值和上面定义的列一一对应

-- ======== 4. 查询 ========
-- 从表格里拿数据
SELECT name, age                    -- 只看名字和年龄两列
FROM student                        -- 从 student 这张表
WHERE age > 18                      -- 只要年龄大于18的
ORDER BY age DESC                   -- 按年龄从大到小排
LIMIT 10;                           -- 最多取10条

-- ======== 5. 删除 ========
DELETE FROM student WHERE id = 1;   -- 删掉学号为1的那一行

-- ======== 6. 改 ========
UPDATE student SET age = 21 WHERE name = '张三';  -- 把张三的年龄改成21

































其他（过程记载）：

sudo apt install mysql-server libmysqlclient-dev

michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo apt install mysql-server libmysqlclient-dev
[sudo] michael-377 的密码： 
正在读取软件包列表... 完成
正在分析软件包的依赖关系树... 完成
正在读取状态信息... 完成                 
将会同时安装下列软件：
  libaio1t64 libcgi-fast-perl libcgi-pm-perl libevent-core-2.1-7t64
  libevent-pthreads-2.1-7t64 libfcgi-bin libfcgi-perl libfcgi0t64
  libhtml-template-perl libmecab2 libmysqlclient21 libzstd-dev mecab-ipadic
  mecab-ipadic-utf8 mecab-utils mysql-client-8.0 mysql-client-core-8.0
  mysql-common mysql-server-8.0 mysql-server-core-8.0
建议安装：
  libipc-sharedcache-perl mailx tinyca
下列【新】软件包将被安装：
  libaio1t64 libcgi-fast-perl libcgi-pm-perl libevent-core-2.1-7t64
  libevent-pthreads-2.1-7t64 libfcgi-bin libfcgi-perl libfcgi0t64
  libhtml-template-perl libmecab2 libmysqlclient-dev libmysqlclient21
  libzstd-dev mecab-ipadic mecab-ipadic-utf8 mecab-utils mysql-client-8.0
  mysql-client-core-8.0 mysql-common mysql-server mysql-server-8.0
  mysql-server-core-8.0
升级了 0 个软件包，新安装了 22 个软件包，要卸载 0 个软件包，有 76 个软件包未被升级。
需要下载 32.3 MB 的归档。
解压缩后会消耗 258 MB 的额外空间。
您希望继续执行吗？ [Y/n] y
0% [执行中]
获取:1 http://cn.archive.ubuntu.com/ubuntu noble/main amd64 mysql-common all 5.8+1.1.0build1 [6,746 B]
获取:2 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 mysql-client-core-8.0 amd64 8.0.46-0ubuntu0.24.04.3 [2,740 kB]
获取:3 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 mysql-client-8.0 amd64 8.0.46-0ubuntu0.24.04.3 [22.4 kB]
获取:4 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 libaio1t64 amd64 0.3.113-6build1.1 [7,210 B]
获取:5 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libevent-core-2.1-7t64 amd64 2.1.12-stable-9ubuntu2 [91.3 kB]
获取:6 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libevent-pthreads-2.1-7t64 amd64 2.1.12-stable-9ubuntu2 [7,982 B]
获取:7 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libmecab2 amd64 0.996-14ubuntu4 [201 kB]
获取:8 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 mysql-server-core-8.0 amd64 8.0.46-0ubuntu0.24.04.3 [17.5 MB]
获取:9 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 mysql-server-8.0 amd64 8.0.46-0ubuntu0.24.04.3 [1,442 kB]
获取:10 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libcgi-pm-perl all 4.63-1 [185 kB]
获取:11 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 libfcgi0t64 amd64 2.4.2-2.1ubuntu0.24.04.1 [27.0 kB]
获取:12 http://cn.archive.ubuntu.com/ubuntu noble/main amd64 libfcgi-perl amd64 0.82+ds-3build2 [21.7 kB]
获取:13 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libcgi-fast-perl all 1:2.17-1 [10.3 kB]
获取:14 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 libfcgi-bin amd64 2.4.2-2.1ubuntu0.24.04.1 [11.2 kB]
获取:15 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 libhtml-template-perl all 2.97-2 [60.2 kB]
获取:16 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 libmysqlclient21 amd64 8.0.46-0ubuntu0.24.04.3 [1,255 kB]
获取:17 http://cn.archive.ubuntu.com/ubuntu noble-updates/main amd64 libzstd-dev amd64 1.5.5+dfsg2-2build1.1 [364 kB]
获取:18 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 libmysqlclient-dev amd64 8.0.46-0ubuntu0.24.04.3 [1,590 kB]
获取:19 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/main amd64 mecab-utils amd64 0.996-14ubuntu4 [4,804 B]
获取:20 http://cn.archive.ubuntu.com/ubuntu noble/main amd64 mecab-ipadic all 2.7.0-20070801+main-3 [6,718 kB]
获取:21 http://cn.archive.ubuntu.com/ubuntu noble/main amd64 mecab-ipadic-utf8 all 2.7.0-20070801+main-3 [4,384 B]
获取:22 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/main amd64 mysql-server all 8.0.46-0ubuntu0.24.04.3 [9,526 B]
已下载 32.3 MB，耗时 12秒 (2,673 kB/s)                                         
正在预设定软件包 ...
正在选中未选择的软件包 mysql-common。
(正在读取数据库 ... 系统当前共安装有 336655 个文件和目录。)
准备解压 .../0-mysql-common_5.8+1.1.0build1_all.deb  ...
正在解压 mysql-common (5.8+1.1.0build1) ...
正在选中未选择的软件包 mysql-client-core-8.0。
准备解压 .../1-mysql-client-core-8.0_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 mysql-client-core-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在选中未选择的软件包 mysql-client-8.0。
准备解压 .../2-mysql-client-8.0_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 mysql-client-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在选中未选择的软件包 libaio1t64:amd64。
准备解压 .../3-libaio1t64_0.3.113-6build1.1_amd64.deb  ...
正在解压 libaio1t64:amd64 (0.3.113-6build1.1) ...
正在选中未选择的软件包 libevent-core-2.1-7t64:amd64。
准备解压 .../4-libevent-core-2.1-7t64_2.1.12-stable-9ubuntu2_amd64.deb  ...
正在解压 libevent-core-2.1-7t64:amd64 (2.1.12-stable-9ubuntu2) ...
正在选中未选择的软件包 libevent-pthreads-2.1-7t64:amd64。
准备解压 .../5-libevent-pthreads-2.1-7t64_2.1.12-stable-9ubuntu2_amd64.deb  ...
正在解压 libevent-pthreads-2.1-7t64:amd64 (2.1.12-stable-9ubuntu2) ...
正在选中未选择的软件包 libmecab2:amd64。
准备解压 .../6-libmecab2_0.996-14ubuntu4_amd64.deb  ...
正在解压 libmecab2:amd64 (0.996-14ubuntu4) ...
正在选中未选择的软件包 mysql-server-core-8.0。
准备解压 .../7-mysql-server-core-8.0_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 mysql-server-core-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在设置 mysql-common (5.8+1.1.0build1) ...
update-alternatives: 使用 /etc/mysql/my.cnf.fallback 来在自动模式中提供 /etc/mys
ql/my.cnf (my.cnf)
正在选中未选择的软件包 mysql-server-8.0。
(正在读取数据库 ... 系统当前共安装有 336879 个文件和目录。)
准备解压 .../00-mysql-server-8.0_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 mysql-server-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在选中未选择的软件包 libcgi-pm-perl。
准备解压 .../01-libcgi-pm-perl_4.63-1_all.deb  ...
正在解压 libcgi-pm-perl (4.63-1) ...
正在选中未选择的软件包 libfcgi0t64:amd64。
准备解压 .../02-libfcgi0t64_2.4.2-2.1ubuntu0.24.04.1_amd64.deb  ...
正在解压 libfcgi0t64:amd64 (2.4.2-2.1ubuntu0.24.04.1) ...
正在选中未选择的软件包 libfcgi-perl。
准备解压 .../03-libfcgi-perl_0.82+ds-3build2_amd64.deb  ...
正在解压 libfcgi-perl (0.82+ds-3build2) ...
正在选中未选择的软件包 libcgi-fast-perl。
准备解压 .../04-libcgi-fast-perl_1%3a2.17-1_all.deb  ...
正在解压 libcgi-fast-perl (1:2.17-1) ...
正在选中未选择的软件包 libfcgi-bin。
准备解压 .../05-libfcgi-bin_2.4.2-2.1ubuntu0.24.04.1_amd64.deb  ...
正在解压 libfcgi-bin (2.4.2-2.1ubuntu0.24.04.1) ...
正在选中未选择的软件包 libhtml-template-perl。
准备解压 .../06-libhtml-template-perl_2.97-2_all.deb  ...
正在解压 libhtml-template-perl (2.97-2) ...
正在选中未选择的软件包 libmysqlclient21:amd64。
准备解压 .../07-libmysqlclient21_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 libmysqlclient21:amd64 (8.0.46-0ubuntu0.24.04.3) ...
正在选中未选择的软件包 libzstd-dev:amd64。
准备解压 .../08-libzstd-dev_1.5.5+dfsg2-2build1.1_amd64.deb  ...
正在解压 libzstd-dev:amd64 (1.5.5+dfsg2-2build1.1) ...
正在选中未选择的软件包 libmysqlclient-dev。
准备解压 .../09-libmysqlclient-dev_8.0.46-0ubuntu0.24.04.3_amd64.deb  ...
正在解压 libmysqlclient-dev (8.0.46-0ubuntu0.24.04.3) ...
正在选中未选择的软件包 mecab-utils。
准备解压 .../10-mecab-utils_0.996-14ubuntu4_amd64.deb  ...
正在解压 mecab-utils (0.996-14ubuntu4) ...
正在选中未选择的软件包 mecab-ipadic。
准备解压 .../11-mecab-ipadic_2.7.0-20070801+main-3_all.deb  ...
正在解压 mecab-ipadic (2.7.0-20070801+main-3) ...
正在选中未选择的软件包 mecab-ipadic-utf8。
准备解压 .../12-mecab-ipadic-utf8_2.7.0-20070801+main-3_all.deb  ...
正在解压 mecab-ipadic-utf8 (2.7.0-20070801+main-3) ...
正在选中未选择的软件包 mysql-server。
准备解压 .../13-mysql-server_8.0.46-0ubuntu0.24.04.3_all.deb  ...
正在解压 mysql-server (8.0.46-0ubuntu0.24.04.3) ...
正在设置 libmecab2:amd64 (0.996-14ubuntu4) ...
正在设置 mysql-client-core-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在设置 libmysqlclient21:amd64 (8.0.46-0ubuntu0.24.04.3) ...
正在设置 libzstd-dev:amd64 (1.5.5+dfsg2-2build1.1) ...
正在设置 libfcgi0t64:amd64 (2.4.2-2.1ubuntu0.24.04.1) ...
正在设置 libcgi-pm-perl (4.63-1) ...
正在设置 libfcgi-bin (2.4.2-2.1ubuntu0.24.04.1) ...
正在设置 libhtml-template-perl (2.97-2) ...
正在设置 mecab-utils (0.996-14ubuntu4) ...
正在设置 libaio1t64:amd64 (0.3.113-6build1.1) ...
正在设置 mysql-client-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在设置 libfcgi-perl (0.82+ds-3build2) ...
正在设置 libevent-core-2.1-7t64:amd64 (2.1.12-stable-9ubuntu2) ...
正在设置 libmysqlclient-dev (8.0.46-0ubuntu0.24.04.3) ...
正在设置 mecab-ipadic (2.7.0-20070801+main-3) ...
Compiling IPA dictionary for Mecab.  This takes long time...
reading /usr/share/mecab/dic/ipadic/unk.def ... 40
emitting double-array: 100% |###########################################| 
/usr/share/mecab/dic/ipadic/model.def is not found. skipped.
reading /usr/share/mecab/dic/ipadic/Symbol.csv ... 208
reading /usr/share/mecab/dic/ipadic/Noun.adjv.csv ... 3328
reading /usr/share/mecab/dic/ipadic/Filler.csv ... 19
reading /usr/share/mecab/dic/ipadic/Noun.nai.csv ... 42
reading /usr/share/mecab/dic/ipadic/Verb.csv ... 130750
reading /usr/share/mecab/dic/ipadic/Postp.csv ... 146
reading /usr/share/mecab/dic/ipadic/Noun.name.csv ... 34202
reading /usr/share/mecab/dic/ipadic/Noun.proper.csv ... 27328
reading /usr/share/mecab/dic/ipadic/Noun.others.csv ... 151
reading /usr/share/mecab/dic/ipadic/Postp-col.csv ... 91
reading /usr/share/mecab/dic/ipadic/Adnominal.csv ... 135
reading /usr/share/mecab/dic/ipadic/Noun.adverbal.csv ... 795
reading /usr/share/mecab/dic/ipadic/Adj.csv ... 27210
reading /usr/share/mecab/dic/ipadic/Prefix.csv ... 221
reading /usr/share/mecab/dic/ipadic/Adverb.csv ... 3032
reading /usr/share/mecab/dic/ipadic/Noun.verbal.csv ... 12146
reading /usr/share/mecab/dic/ipadic/Noun.org.csv ... 16668
reading /usr/share/mecab/dic/ipadic/Noun.number.csv ... 42
reading /usr/share/mecab/dic/ipadic/Suffix.csv ... 1393
reading /usr/share/mecab/dic/ipadic/Noun.csv ... 60477
reading /usr/share/mecab/dic/ipadic/Noun.place.csv ... 72999
reading /usr/share/mecab/dic/ipadic/Others.csv ... 2
reading /usr/share/mecab/dic/ipadic/Auxil.csv ... 199
reading /usr/share/mecab/dic/ipadic/Noun.demonst.csv ... 120
reading /usr/share/mecab/dic/ipadic/Conjunction.csv ... 171
reading /usr/share/mecab/dic/ipadic/Interjection.csv ... 252
emitting double-array: 100% |###########################################| 
reading /usr/share/mecab/dic/ipadic/matrix.def ... 1316x1316
emitting matrix      : 100% |###########################################| 

done!
update-alternatives: 使用 /var/lib/mecab/dic/ipadic 来在自动模式中提供 /var/lib/
mecab/dic/debian (mecab-dictionary)
正在设置 libcgi-fast-perl (1:2.17-1) ...
正在设置 libevent-pthreads-2.1-7t64:amd64 (2.1.12-stable-9ubuntu2) ...
正在设置 mysql-server-core-8.0 (8.0.46-0ubuntu0.24.04.3) ...
正在设置 mecab-ipadic-utf8 (2.7.0-20070801+main-3) ...
Compiling IPA dictionary for Mecab.  This takes long time...
reading /usr/share/mecab/dic/ipadic/unk.def ... 40
emitting double-array: 100% |###########################################| 
/usr/share/mecab/dic/ipadic/model.def is not found. skipped.
reading /usr/share/mecab/dic/ipadic/Symbol.csv ... 208
reading /usr/share/mecab/dic/ipadic/Noun.adjv.csv ... 3328
reading /usr/share/mecab/dic/ipadic/Filler.csv ... 19
reading /usr/share/mecab/dic/ipadic/Noun.nai.csv ... 42
reading /usr/share/mecab/dic/ipadic/Verb.csv ... 130750
reading /usr/share/mecab/dic/ipadic/Postp.csv ... 146
reading /usr/share/mecab/dic/ipadic/Noun.name.csv ... 34202
reading /usr/share/mecab/dic/ipadic/Noun.proper.csv ... 27328
reading /usr/share/mecab/dic/ipadic/Noun.others.csv ... 151
reading /usr/share/mecab/dic/ipadic/Postp-col.csv ... 91
reading /usr/share/mecab/dic/ipadic/Adnominal.csv ... 135
reading /usr/share/mecab/dic/ipadic/Noun.adverbal.csv ... 795
reading /usr/share/mecab/dic/ipadic/Adj.csv ... 27210
reading /usr/share/mecab/dic/ipadic/Prefix.csv ... 221
reading /usr/share/mecab/dic/ipadic/Adverb.csv ... 3032
reading /usr/share/mecab/dic/ipadic/Noun.verbal.csv ... 12146
reading /usr/share/mecab/dic/ipadic/Noun.org.csv ... 16668
reading /usr/share/mecab/dic/ipadic/Noun.number.csv ... 42
reading /usr/share/mecab/dic/ipadic/Suffix.csv ... 1393
reading /usr/share/mecab/dic/ipadic/Noun.csv ... 60477
reading /usr/share/mecab/dic/ipadic/Noun.place.csv ... 72999
reading /usr/share/mecab/dic/ipadic/Others.csv ... 2
reading /usr/share/mecab/dic/ipadic/Auxil.csv ... 199
reading /usr/share/mecab/dic/ipadic/Noun.demonst.csv ... 120
reading /usr/share/mecab/dic/ipadic/Conjunction.csv ... 171
reading /usr/share/mecab/dic/ipadic/Interjection.csv ... 252
emitting double-array: 100% |###########################################| 
reading /usr/share/mecab/dic/ipadic/matrix.def ... 1316x1316
emitting matrix      : 100% |###########################################| 

done!
update-alternatives: 使用 /var/lib/mecab/dic/ipadic-utf8 来在自动模式中提供 /var
/lib/mecab/dic/debian (mecab-dictionary)
正在设置 mysql-server-8.0 (8.0.46-0ubuntu0.24.04.3) ...
update-alternatives: 使用 /etc/mysql/mysql.cnf 来在自动模式中提供 /etc/mysql/my.
cnf (my.cnf)
Renaming removed key_buffer and myisam-recover options (if present)
mysqld will log errors to /var/log/mysql/error.log
mysqld is running as pid 52174
Created symlink /etc/systemd/system/multi-user.target.wants/mysql.service → /usr
/lib/systemd/system/mysql.service.
正在设置 mysql-server (8.0.46-0ubuntu0.24.04.3) ...
正在处理用于 man-db (2.12.0-4build2) 的触发器 ...
正在处理用于 libc-bin (2.39-0ubuntu8.8) 的触发器 ...


sudo mysql

1. 建数据库：

CREATE DATABASE chatroom;

2. 给聊天室建一个专用 MySQL 用户（别用 root）：

CREATE USER 'chat'@'127.0.0.1' IDENTIFIED BY 'chat123';

- 'chat' 是用户名
- '127.0.0.1' 限制这个用户只能从本机连
- 'chat123' 是密码，你自己换一个也成

3. 把 chatroom 数据库的权限给这个用户：

GRANT ALL PRIVILEGES ON chatroom.* TO 'chat'@'127.0.0.1';
FLUSH PRIVILEGES;
























michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo mysql
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 8
Server version: 8.0.46-0ubuntu0.24.04.3 (Ubuntu)

Copyright (c) 2000, 2026, Oracle and/or its affiliates.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> CREATE DATABASE chatroom
    -> CREATE USER 'chat'@'127.0.0.1' IDENTIFIED BY '2643534502';
ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'CREATE USER 'chat'@'127.0.0.1' IDENTIFIED BY '2643534502'' at line 2
mysql> GRANT ALL PRIVILEGES ON chatroom.* TO 'chat'@'127.0.0.1';
ERROR 1410 (42000): You are not allowed to create a user with GRANT
mysql> GRANT ALL PRIVILEGES ON chatroom.* TO 'chat'@'127.0.0.1';  FLUSH PRIVILEGES;
ERROR 1410 (42000): You are not allowed to create a user with GRANT
Query OK, 0 rows affected (0.00 sec)

mysql> CREATE DATABASE chatroom;
Query OK, 1 row affected (0.02 sec)

mysql> GRANT ALL PRIVILEGES ON chatroom.* TO 'chat'@'127.0.0.1';
ERROR 1410 (42000): You are not allowed to create a user with GRANT
mysql> CREATE USER 'chat'@'127.0.0.1' IDENTIFIED BY '2643534502';
Query OK, 0 rows affected (0.04 sec)

mysql> GRANT ALL PRIVILEGES ON chatroom.* TO 'chat'@'127.0.0.1';
Query OK, 0 rows affected (0.01 sec)

mysql> FLUSH PRIVILEGES;
Query OK, 0 rows affected (0.01 sec)

mysql> 






mysql -u chat -p -h 127.0.0.1 chatroomG