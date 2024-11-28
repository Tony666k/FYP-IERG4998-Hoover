#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5001 // 设置UDP端口
#define BUF_SIZE 1024 // 设置缓冲区大小
#define MESSAGE_LENGTH 4 // 每个数据包的长度 (4个'1')

// 固定消息
#define FIXED_MSG "1111" // 固定的4个'1'

// 发送UDP消息的函数
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

// 线程函数：向10000端口发送2个固定数据包 "1111"
void* send_to_10000(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 发送两个"1111"的数据包到10000端口
    send_udpmsg(server_ip, 10000, client_ip, client_port, FIXED_MSG);
    send_udpmsg(server_ip, 10000, client_ip, client_port, FIXED_MSG);

    return NULL;
}

// 线程函数：向10001端口发送2个固定数据包 "1111"
void* send_to_10001(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 发送两个"1111"的数据包到10001端口
    send_udpmsg(server_ip, 10001, client_ip, client_port, FIXED_MSG);
    send_udpmsg(server_ip, 10001, client_ip, client_port, FIXED_MSG);

    return NULL;
}

// 线程函数：计算XOR结果并发送到10002端口
void* send_to_10002(void *arg) {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 固定消息 "1111" XOR 运算：对两个数据包进行 XOR
    char XOR_result[MESSAGE_LENGTH + 1]; // 存储XOR结果

    // XOR: "1111" XOR "1111" = "0000"
    for (int i = 0; i < MESSAGE_LENGTH; i++) {
        XOR_result[i] = (FIXED_MSG[i] == FIXED_MSG[i]) ? '0' : '1'; // 因为是相同的"1111"，结果是"0000"
    }
    XOR_result[MESSAGE_LENGTH] = '\0'; // 添加结束符

    // 发送XOR结果到10002端口
    send_udpmsg(server_ip, 10002, client_ip, client_port, XOR_result);

    return NULL;
}

int main() {
    const char *server_ip = "192.168.229.135"; // 服务器IP地址
    const char *client_ip = "192.168.229.134"; // 客户端IP地址
    int client_port = PORT;

    // 发送4次消息
    for (int i = 0; i < 4; i++) {
        pthread_t thread_10000, thread_10001, thread_10002;

        // 创建线程分别发送数据到10000, 10001
        pthread_create(&thread_10000, NULL, send_to_10000, NULL);
        pthread_create(&thread_10001, NULL, send_to_10001, NULL);

        // 计算XOR并发送到10002
        pthread_create(&thread_10002, NULL, send_to_10002, NULL);

        // 等待线程完成
        pthread_join(thread_10000, NULL);  // 等待thread_10000完成
        pthread_join(thread_10001, NULL);  // 等待thread_10001完成
        pthread_join(thread_10002, NULL);  // 等待thread_10002完成

        // 可选：添加小延时
        // sleep(1);
    }

    return 0;
}


