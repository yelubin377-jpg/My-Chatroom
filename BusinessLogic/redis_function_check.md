```c++
 //建立到redis服务器的TCP连接，内部调 redisConnect("127.0.0.1", 6379)，返回的 redisContext* 存到 _ctx。所有后续操作都靠这个 _ctx 发命令。
    bool connect();     //already used（打钩）
    //HSET key field value  HASH表里设一个字段，注册时存用户信息
    bool hset(const std::string& key,const std::string& field,const std::string& value); //already used(大狗)
    //SET key value 存一个键值对，账号系统里存token 
    bool set(const std::string& key,const std::string& value); //already used (打钩）
    //DEL key(删除一个key)
    bool del(const std::string& key);  //already be written
    //HDEL key field 删除哈希表里的一个字段
    bool hdel(const std::string& key , const std::string& field);//alredy ok
    //EXISTS key(判断key是否存在)
    bool exists(const std::string& key); // already completed
    //EXPIRE key seconds 设TTL过期时间，登录后设置token有效期eg:EXPIRE token:abc123          		86400(24小时)
    bool expire(const std::string& key , int seconds); // already finished
    //SADD key member 添加好友    eg：SADD friends:1001 1002(往1001的好友列表里加1002)
    bool sadd(const std::string& key ,  const std::string& member); //already ready
    //SREM key member 删除一个好友 eg:SREM key member
    bool srem(const std::string& key , const std::string& member); //already already
    //LPUSH key value从列表左边推入一个元素)。(离线消息存储：LPUSH offline:1001 msg_json)
    bool lpush(const std::string& key , const std::string& value);//ready already
    //LTRIM key start stop
    bool ltrim(const std::string& key, int start , int stop); //already

	//HGET key field HASH表里取一个字段（登录时查密码：HGET user:1001 password）
	std::string hget(const std::string& key ,const std::string& field); //already used
	//GET key 取字符串，登录时根据token查uid，GET token:abc123
	std::string get(const std::string& key);//alredy
	//INCR key
	std::string incr(const std::string& key);

	//下面两个函数需要区分是否存在
	//SISMEMBER key member 判断元素是否在集合里，发私聊前面判断是不是好友 
	bool sismember(const std::string& key,const std::string& member); //dy
	//HEXISTS key field 判断Hash里某字段是否存在，注册是查用户名是否被占用HEXISTS 				user:username michael
	bool hexists(const std::string& key , const std::string& field);  //rea

	//SMEMBERS key 返回集合全部元素 - 获取好友列表的时候用 or 群成员略表是用
	std::vector<std::string> smembers(const std::string& key);  // alre
	//LRANGE key start stop取出列表的一段，人话：把离线消息拉出来 LRANGE offline:1001 0 -1(全	部)
	std::vector<std::string> lrange(const std::string& key , int start, int 			stop);//ady
	//LLEN key 返回从列表长度 eg：查离线消息有几条：LLEN offline:1001
	int llen(const std::string& key);              
```

