1.         read                                        vs                                                   readv
普通 read 一次只能读进一个缓冲区：                                                                          
char buf[100];
read(fd, buf, 100);// 只能读进 buf 这一块  
                                                                                            readv（v = vector）一次能读进多个缓冲区：
                                                                                                     struct iovec vec[2];
                                                                                                     vec[0].iov_base = 第一块内存;
                                                                                                     vec[0].iov_len  = 第一块多大;
                                                                                                     vec[1].iov_base = 第二块内存;
                                                                                                     vec[1].iov_len  = 第二块多大;
                                                                                                     readv(fd, vec, 2);   // 一次系统调用，先填 vec[0]，满了接着填 vec[1]




2.为什么不用"先 ensureWritable 扩容，再 read"这种简单做法？
因为那样要两次系统调用：先扩容，再 read。而且你每次 read 前都得判断"空间够不够"，扩容可能要拷贝数据（很贵）。
readv 的做法是：一次系统调用 + 栈上临时缓冲区兜底，把数据全部读出来，之后才统一处理扩容。省了一次 read，也避免了"读一半发现空间不够"的尴尬。



3.怎么样指向_buffer首地址 ？ 直接_buffer.begin() ? 不行,不是裸指针     复盘时我可以想想:为什么不行？为什么要是裸指针?
于是乎: *_buffer.size() -》 解引用，拿到第一元素本体，但我要的是地址  -》 &*_buffer这个时候就拿到了


4.std::copy(起点, 终点, 目标) = 把 [起点, 终点) 的数据复制到目标位置。
5.assert（0） -》 崩溃

6.static_cast强制转换review
    编译器帮你检查，转换更安全  -  显眼  

7.深拷贝 vs   浅拷贝   review
                    假设有个结构体，里面有个指针：

                    struct Msg 
                    {
                        char* data;   // 指向堆上的一块数据
                        int len;
                    };
                    Msg a;
                    a.data = new char[100];   // a.data 指向堆上 100 字节
                    strcpy(a.data, "hello");
                    现在 Msg b = a;（拷贝 a 给 b）。这个拷贝有两种做法：

                |                       浅拷贝                                         |                                  深拷贝                                 |     

                            // 默认的拷贝就是浅拷贝                                                                                 
                            Msg b = a;
                结果：

                            a.data ──→ [堆上的 "hello"]
                                                ↑
                                                ↑
                                  b.data ───────┘   两个指针指向同一块内存！

                a.data 和 b.data 都是同一个地址。问题来了：

                // 如果 b 析构时 delete 了 data
                delete b.data;      // 堆上的 "hello" 被释放

                // 现在 a.data 变成了野指针！
                a.data;             // 指向已释放的内存，崩
                                                                                                    Msg deepCopy(const Msg& a)
                                                                                                    {
                                                                                                        Msg b;
                                                                                                        b.data = new char[a.len];
                                                                                                        memcpy(b.data , a.data , a.len);
                                                                                                        b.len = a.len;
                                                                                                        return b;
                                                                                                    }
                                                                                            




