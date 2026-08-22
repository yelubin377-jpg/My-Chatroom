michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo apt install libhiredis-dev
[sudo] michael-377 的密码： 
正在读取软件包列表... 完成
正在分析软件包的依赖关系树... 完成
正在读取状态信息... 完成                 
将会同时安装下列软件：
  libhiredis1.1.0
下列【新】软件包将被安装：
  libhiredis-dev libhiredis1.1.0
升级了 0 个软件包，新安装了 2 个软件包，要卸载 0 个软件包，有 76 个软件包未被升级。
需要下载 120 kB 的归档。
解压缩后会消耗 485 kB 的额外空间。
您希望继续执行吗？ [Y/n] y
获取:1 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/universe amd64 libhiredis1.1.0 amd64 1.2.0-6ubuntu3 [41.4 kB]
获取:2 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/universe amd64 libhiredis-dev amd64 1.2.0-6ubuntu3 [78.3 kB]
已下载 120 kB，耗时 1秒 (95.1 kB/s)      
正在选中未选择的软件包 libhiredis1.1.0:amd64。
(正在读取数据库 ... 系统当前共安装有 336550 个文件和目录。)
准备解压 .../libhiredis1.1.0_1.2.0-6ubuntu3_amd64.deb  ...
正在解压 libhiredis1.1.0:amd64 (1.2.0-6ubuntu3) ...
正在选中未选择的软件包 libhiredis-dev:amd64。
准备解压 .../libhiredis-dev_1.2.0-6ubuntu3_amd64.deb  ...
正在解压 libhiredis-dev:amd64 (1.2.0-6ubuntu3) ...
正在设置 libhiredis1.1.0:amd64 (1.2.0-6ubuntu3) ...
正在设置 libhiredis-dev:amd64 (1.2.0-6ubuntu3) ...
正在处理用于 libc-bin (2.39-0ubuntu8.7) 的触发器 ...
michael-377@michael-377-Legion-Y9000P-IAX10:~$ redis-server --daemonize yes
找不到命令 “redis-server”，但可以通过以下软件包安装它：
sudo apt install redis-server
michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo apt install redis-server
正在读取软件包列表... 完成
正在分析软件包的依赖关系树... 完成
正在读取状态信息... 完成                 
将会同时安装下列软件：
  libjemalloc2 liblzf1 redis-tools
建议安装：
  ruby-redis
下列【新】软件包将被安装：
  libjemalloc2 liblzf1 redis-server redis-tools
升级了 0 个软件包，新安装了 4 个软件包，要卸载 0 个软件包，有 76 个软件包未被升级。
需要下载 1,482 kB 的归档。
解压缩后会消耗 7,568 kB 的额外空间。
您希望继续执行吗？ [Y/n] y
0% [执行中]
获取:1 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/universe amd64 libjemalloc2 amd64 5.3.0-2build1 [256 kB]
获取:2 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble/universe amd64 liblzf1 amd64 3.6-4 [7,624 B]
获取:3 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/universe amd64 redis-tools amd64 5:7.0.15-1ubuntu0.24.04.4 [1,166 kB]
获取:4 http://mirrors.tuna.tsinghua.edu.cn/ubuntu noble-updates/universe amd64 redis-server amd64 5:7.0.15-1ubuntu0.24.04.4 [51.7 kB]
已下载 1,482 kB，耗时 2秒 (625 kB/s)    
正在选中未选择的软件包 libjemalloc2:amd64。
(正在读取数据库 ... 系统当前共安装有 336610 个文件和目录。)
准备解压 .../libjemalloc2_5.3.0-2build1_amd64.deb  ...
正在解压 libjemalloc2:amd64 (5.3.0-2build1) ...
正在选中未选择的软件包 liblzf1:amd64。
准备解压 .../liblzf1_3.6-4_amd64.deb  ...
正在解压 liblzf1:amd64 (3.6-4) ...
正在选中未选择的软件包 redis-tools。
准备解压 .../redis-tools_5%3a7.0.15-1ubuntu0.24.04.4_amd64.deb  ...
正在解压 redis-tools (5:7.0.15-1ubuntu0.24.04.4) ...
正在选中未选择的软件包 redis-server。
准备解压 .../redis-server_5%3a7.0.15-1ubuntu0.24.04.4_amd64.deb  ...
正在解压 redis-server (5:7.0.15-1ubuntu0.24.04.4) ...
正在设置 libjemalloc2:amd64 (5.3.0-2build1) ...
正在设置 liblzf1:amd64 (3.6-4) ...
正在设置 redis-tools (5:7.0.15-1ubuntu0.24.04.4) ...
正在设置 redis-server (5:7.0.15-1ubuntu0.24.04.4) ...
Created symlink /etc/systemd/system/redis.service → /usr/lib/systemd/system/redi
s-server.service.
Created symlink /etc/systemd/system/multi-user.target.wants/redis-server.service
 → /usr/lib/systemd/system/redis-server.service.
正在处理用于 man-db (2.12.0-4build2) 的触发器 ...
正在处理用于 libc-bin (2.39-0ubuntu8.7) 的触发器 ...
michael-377@michael-377-Legion-Y9000P-IAX10:~$ redis-server --daemonize yes
michael-377@michael-377-Legion-Y9000P-IAX10:~$ redis-cli ping
PONG
michael-377@michael-377-Legion-Y9000P-IAX10:~$



