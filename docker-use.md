michael-377@michael-377-Legion-Y9000P-IAX10:~$ curl -fsSL https://get.docker.com | sudo sh
[sudo] michael-377 的密码： 
# Executing docker install script, commit: a23123f03978989e95d257beb9de0c5ad9da6e70
+ sh -c apt-get -qq update >/dev/null
W: 无法下载 https://downloads.typora.io/linux/./InRelease  无法发起与 downloads.typora.io:443 (2001::6ca0:a50b) 的连接 - connect (101: 网络不可达) 无法连接上 downloads.typora.io:443 (31.13.69.169)，连接超时
W: 部分索引文件下载失败。如果忽略它们，那将转而使用旧的索引文件。
+ sh -c DEBIAN_FRONTEND=noninteractive apt-get -y -qq install ca-certificates curl >/dev/null
+ sh -c install -m 0755 -d /etc/apt/keyrings
+ sh -c curl -fsSL "https://download.docker.com/linux/ubuntu/gpg" -o /etc/apt/keyrings/docker.asc
+ sh -c chmod a+r /etc/apt/keyrings/docker.asc
+ sh -c echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu noble stable" > /etc/apt/sources.list.d/docker.list
+ sh -c apt-get -qq update >/dev/null
W: 无法下载 https://downloads.typora.io/linux/./InRelease  无法发起与 downloads.typora.io:443 (2001::6ca0:a50b) 的连接 - connect (101: 网络不可达) 无法连接上 downloads.typora.io:443 (31.13.69.169)，连接超时
W: 部分索引文件下载失败。如果忽略它们，那将转而使用旧的索引文件。
+ apt_flags=-y -qq
+ [ -n  ]
+ sh -c DEBIAN_FRONTEND=noninteractive apt-get -y -qq install docker-ce docker-ce-cli containerd.io docker-compose-plugin docker-ce-rootless-extras docker-buildx-plugin docker-model-plugin >/dev/null
Using systemd to manage Docker service
+ sh -c systemctl enable --now docker.service 2>/dev/null
INFO: Docker daemon enabled and started

+ sh -c docker version
Client: Docker Engine - Community
 Version:           29.7.2
 API version:       1.55
 Go version:        go1.26.5
 Git commit:        a7dcaa6
 Built:             Wed Aug  5 18:28:53 2026
 OS/Arch:           linux/amd64
 Context:           default

Server: Docker Engine - Community
 Engine:
  Version:          29.7.2
  API version:      1.55 (minimum version 1.40)
  Go version:       go1.26.5
  Git commit:       6a43e3d
  Built:            Wed Aug  5 18:28:53 2026
  OS/Arch:          linux/amd64
  Experimental:     false
 containerd:
  Version:          v2.3.3
  GitCommit:        aad11006b869517fcd3009450b6f82da282e1a9b
 runc:
  Version:          1.4.3
  GitCommit:        v1.4.3-0-gbb14dabe
 docker-init:
  Version:          0.19.0
  GitCommit:        de40ad0

================================================================================

To run Docker as a non-privileged user, consider setting up the
Docker daemon in rootless mode for your user:

    dockerd-rootless-setuptool.sh install

Visit https://docs.docker.com/go/rootless/ to learn about rootless mode.


To run the Docker daemon as a fully privileged service, but granting non-root
users access, refer to https://docs.docker.com/go/daemon-access/

WARNING: Access to the remote API on a privileged Docker daemon is equivalent
         to root access on the host. Refer to the 'Docker daemon attack surface'
         documentation for details: https://docs.docker.com/go/attack-surface/

================================================================================

michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker --version
Docker version 29.7.2, build a7dcaa6
michael-377@michael-377-Legion-Y9000P-IAX10:~$ 


michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker --version
Docker version 29.7.2, build a7dcaa6
michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker ps
permission denied while trying to connect to the docker API at unix:///var/run/docker.sock
michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo usermod -aG docker $USER
[sudo] michael-377 的密码： 
michael-377@michael-377-Legion-Y9000P-IAX10:~$ 

michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker --version
Docker version 29.7.2, build a7dcaa6
michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker ps
permission denied while trying to connect to the docker API at unix:///var/run/docker.sock
michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo usermod -aG docker $USER
[sudo] michael-377 的密码： 
michael-377@michael-377-Legion-Y9000P-IAX10:~$ sudo usermod -aG docker $USER
michael-377@michael-377-Legion-Y9000P-IAX10:~$ newgrp docker
michael-377@michael-377-Legion-Y9000P-IAX10:~$ docker ps
CONTAINER ID   IMAGE     COMMAND   CREATED   STATUS    PORTS     NAMES
michael-377@michael-377-Legion-Y9000P-IAX10:~$ 





sudo docker run -it --rm chatroom-client 10.30.1.235 2026
docker save chatroom-client -o chatroom-client.tar
docker load -i chatroom-client.tar
sudo docker run -it --rm chatroom-client 10.30.1.235 2026
