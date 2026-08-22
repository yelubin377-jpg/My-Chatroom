离线消息: 存 - 推 -  清
存可以存在mysql ， 再制一张表专门存，然后一套固定打法 ， 除了判断是conn之外区别或许不大（还没写，里清楚思路ing）
判断conn连接上了就推信息给他，推一条清理一条

参照表格，思考可调用的已经写好的函数：
    判断他到底再不在线上:GetconnByUser
    存消息？ mysql的savehistory（不过估计要开新的column ， 暂且叫他offline message ， 或者叫作OM也行
    怎么推？上线后推 ， 那我怎么知道他上线了？LoginHandler，一上线，再看到status ok 附近直接推消息
    怎么删除？ mysql应该有相应的删除函数去找找 -》 MYSQL DELETE

