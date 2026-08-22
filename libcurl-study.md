AI 快速过一遍基本概念与框架
libcurl 
    句柄（CURL*） - 所有
    - 输入 ：URL - 发给谁
            HTTP 
            Header
            Body
    - 输出 ：回调函数
            writecallback - 数据来了往...放
    - 执行 ：curl_easy_perform() 阻塞，干完再回来

