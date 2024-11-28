#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5001 //set the udp port
#define BUF_SIZE 1024 //set the buffer size 
#define MESSAGE_LENGTH 16 //define the total msg length (16 bits)

// 固定的二进制消息
#define FIXED_MSG "1111111111111111" // 固定的16个1

// 将固定二进制消息分割成每4个1为一个小数据包
void split_message(const char *msg, char *part1, char *part2, char *part3, char *part4) {
    strncpy(part1, msg, 4);
    part1[4] = '\0';
    
    strncpy(part2, msg + 4, 4);
    part2[4] = '\0';
    
    strncpy(part3, msg + 8, 4);
    part3[4] = '\0';
    
    strncpy(part4, msg + 12, 4);
    part4[4] = '\0';
}

// 发送UDP消息
int send_udpmsg(const char *server_ip, int server_port, const char *client_ip, int client_port, const char *message) {
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;

    // 创建socket
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sock_descriptor);
        return -1;
    }
    server_addr.sin_port = htons(server_port);

    // 绑定端口
    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return -1;
    }

    // 设置客户端地址
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, client_ip, &client_addr.sin_addr) <= 0) {
        perror("Invalid client IP address");
        close(sock_descriptor);
        return -1;
    }
    client_addr.sin_port = htons(client_port);

    // 发送数据包
    if (sendto(sock_descriptor, message, strlen(message), 0, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Send failed");
        close(sock_descriptor);
        return -1;
    }

    printf("Message sent: %s\n", message);
    close(sock_descriptor);
    return 0;
}

// 线程函数：向10000端口发送4个1的数据包
void* send_to_10000(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 固定二进制消息分割
    char part1[5], part2[5], part3[5], part4[5];
    split_message(FIXED_MSG, part1, part2, part3, part4);

    // 发送每个分包
    send_udpmsg(server_ip, 10000, client_ip, client_port, part1);
    send_udpmsg(server_ip, 10000, client_ip, client_port, part2);
    send_udpmsg(server_ip, 10000, client_ip, client_port, part3);
    send_udpmsg(server_ip, 10000, client_ip, client_port, part4);

    return NULL;
}

// 线程函数：向10001端口发送4个1的数据包
void* send_to_10001(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 固定二进制消息分割
    char part1[5], part2[5], part3[5], part4[5];
    split_message(FIXED_MSG, part1, part2, part3, part4);

    // 发送每个分包
    send_udpmsg(server_ip, 10001, client_ip, client_port, part1);
    send_udpmsg(server_ip, 10001, client_ip, client_port, part2);
    send_udpmsg(server_ip, 10001, client_ip, client_port, part3);
    send_udpmsg(server_ip, 10001, client_ip, client_port, part4);

    return NULL;
}

// 线程函数：向10002端口发送XOR结果
void* send_to_10002(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 固定二进制消息分割
    char part1[5], part2[5], part3[5], part4[5];
    split_message(FIXED_MSG, part1, part2, part3, part4);

    // XOR操作：在这里我们假设 XOR 是简单的二进制异或运算
    char XOR_result[5];
    for (int i = 0; i < 4; i++) {
        // 假设 XOR 的操作是基于字符的
        XOR_result[i] = (part1[i] == part2[i]) ? '0' : '1'; // XOR 第一个和第二个部分
    }
    XOR_result[4] = '\0'; // 添加字符串结束符

    // 发送XOR结果
    send_udpmsg(server_ip, 10002, client_ip, client_port, XOR_result);

    return NULL;
}

int main() {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 创建共享数据结构保存消息（此处不用再使用原结构）
    
    // 发送10次消息
    for (int i = 0; i < 10; i++) {
        pthread_t thread_10000, thread_10001, thread_10002;

        // 创建线程分别发送数据到10000, 10001
        pthread_create(&thread_10000, NULL, send_to_10000, NULL);
        pthread_create(&thread_10001, NULL, send_to_10001, NULL);

        pthread_join(thread_10000, NULL);  // 等待thread_10000完成
        pthread_join(thread_10001, NULL);  // 等待thread_10001完成

        // 发送XOR结果到10002
        pthread_create(&thread_10002, NULL, send_to_10002, NULL);
        pthread_join(thread_10002, NULL); // 等待thread_10002完成

        // 可选：添加小延时
        // sleep(1);
    }

    return 0;
}

