#pragma once
#include <string>
#include <cctype>

class base64 {
private:
    // base64字符映射表（64个可打印ASCII字符）
    static const std::string base64_chars;
    
    /**
     * 验证字符是否为有效Base64字符
     * @param c 待验证字符
     * @return 是否为有效字符
     */
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

public:
    /**
     * Base64编码函数
     * @param input 原始字符串
     * @return Base64编码结果
     */
    static std::string encode(const std::string& input) {
        std::string output;  // 输出字符串
        int i = 0, j = 0;
        unsigned char char_array_3[3];  // 存储3字节原始数据
        unsigned char char_array_4[4];  // 存储4个6位编码单元
        size_t length = input.length();
        const char* bytes_to_encode = input.c_str();
        
        // 遍历输入字节流
        while (length--) {
            char_array_3[i++] = *(bytes_to_encode++);
            
            // 每积累3字节进行编码
            if (i == 3) {
                // 拆分3字节为4个6位单元
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + 
                                 ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + 
                                 ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                // 将6位单元映射为Base64字符
                for (i = 0; i < 4; i++) 
                    output += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        // 处理剩余字节（不足3字节情况）
        if (i > 0) {
            // 补零操作
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';
                
            // 拆分并映射
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + 
                             ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + 
                             ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            // 添加有效字符
            for (j = 0; j < i + 1; j++)
                output += base64_chars[char_array_4[j]];
            
            // 补充等号填充
            while (i++ < 3)
                output += '=';
        }
        
        return output;
    }
    
    /**
     * Base64解码函数
     * @param input Base64编码字符串
     * @return 原始字符串
     */
    static std::string decode(const std::string& input) {
        std::string output;  // 输出字符串
        int i = 0, j = 0, in_ = 0;
        unsigned char char_array_4[4];  // 存储4个6位编码单元
        unsigned char char_array_3[3];  // 存储3字节原始数据
        size_t length = input.length();
        
        // 遍历Base64字符串
        while (length-- && input[in_] != '=' && is_base64(input[in_])) {
            char_array_4[i++] = input[in_]; in_++;
            
            // 每积累4字符进行解码
            if (i == 4) {
                // 字符映射回6位值
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                
                // 重组4个6位单元为3字节
                char_array_3[0] = (char_array_4[0] << 2) + 
                                 ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + 
                                 ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + 
                                 char_array_4[3];
                
                // 添加到输出
                for (i = 0; i < 3; i++)
                    output += char_array_3[i];
                i = 0;
            }
        }
        
        // 处理剩余字符
        if (i > 0) {
            // 补零操作
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;
                
            // 字符映射回6位值
            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            
            // 重组为原始字节
            char_array_3[0] = (char_array_4[0] << 2) + 
                             ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + 
                             ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + 
                             char_array_4[3];
            
            // 添加有效字节
            for (j = 0; j < i - 1; j++)
                output += char_array_3[j];
        }
        
        return output;
    }
};
// 初始化静态成员变量
inline const std::string base64::base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789+/";