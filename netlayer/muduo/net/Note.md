1.shared_ptr && weak_ptr  再强化
    shared_ptr — 共享所有权
    问题： 一个 new 出来的对象，谁负责 delete？
    //没有 shared_ptr 的时候——你永远不确定能不能删
    TcpConnection* conn = new TcpConnection(...);
    _conns.insert(conn);    // 在线列表指着它
    // 用户断开了 → 你该 delete conn 吗？
    // 但 Handler 里还在用它！
    // 删了 → 野指针，崩
    // 不删 → 内存泄漏

    shared_ptr 的本质：多一个人看着我，我就活着。最后一个看我的走了，我自动死。
    缺点：可能会循环引用，自己指向自己，永远关不掉
    如果 tie 用 shared_ptr:
     TcpConnection → Channel → shared_ptr → TcpConnection
        |                                           |
        ----───────────────────────────────────-----            内存泄漏！


更具体一点：
第一步：正常活着
TcpServer::_conns ──shared_ptr──→ [TcpConnection] 引用计数 = 1
                                        │
                                        │ (拥有, unique_ptr)
                                        ↓
                                   [Channel]
                                        │
                                        │ (_tie 如果是 shared_ptr)
                                        │
                                        └──shared_ptr──→ [TcpConnection] 引用计数 = 2
                                              这个指向了自己！

两个箭头都是从 TcpConnection 出发，回到了 TcpConnection 自己。引用计数是 2——TcpServer 指着它、它内部的 Channel 又指着它。

---
第二步：TcpServer 放手

TcpServer::_conns ──XX (移除)

[TcpConnection] 引用计数 = 1   ← 谁还在指？
     │
     ↓
[Channel]
     │
     └──shared_ptr──→ [TcpConnection]  ← Channel 还指着自己
                          ↑
                    引用计数 = 1，不能析构

TcpServer 走了，但引用计数是 1 不是 0。TcpConnection 活着，Channel 也活着，_tie 也活着。但外面没有人能访问这块内存了——TcpServer 的 _conns 已经把它擦掉了。这块内存变成了孤岛。

---
第三步：困死

[TcpConnection] 引用计数永远 = 1
     │
     │  不能析构 ←── 引用计数不是 0
     │
     ↓
[Channel] 不能析构 ←── TcpConnection 不死它就不死
     │
     │  _tie 不能释放 ←── Channel 不死它就不释放
     │
     └──→ 引用计数永远不会变成 0 ←── 死循环

    





        weak_ptr — 看，但不保活                            vs                           普通指针
         weak_ptr：能问"还活着吗"                                                  普通指针：死了还指着
weak_ptr 不能独立存在。它依赖 shared_ptr 创建的控制块。
        开销——控制块占内存，lock() 是原子操作






2.CPU 原子操作（多线程上下文）

    原子操作就是：CPU 保证这一行直接完成，中间不被打断：

    std::atomic<int> count(0);
    
    // 线程A: 原子操作，三步合一，一个 CPU 指令完成
    count.fetch_add(1);    // count 从 5 变成 6

    // 线程B: 同时到达，但 A 的操作对 B 完全不可见中间状态
    count.fetch_add(1);    // count 从 6 变成 7  ← 对的
