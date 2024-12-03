#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define BUF_SIZE 1024       // 設置緩衝區大小
#define MESSAGE_LENGTH 8    // 定義消息長度

// 生成指定長度的隨機二進制消息
void genrandom_binary(char *msg, int len) {
    for (int i = 0; i < len; i++)
        msg[i] = rand() % 2 ? '1' : '0'; // 隨機選擇 '1' 或 '0' 生成消息字符串
    msg[len] = '\0';  // 添加結束符
}

// 使用 XOR 操作對兩個二進制消息進行處理
void XOR_msg(const char *msg1, const char *msg2, char *result, int len) {
    for (int i = 0; i < len; i++) {
        // 當兩個字符相同時結果為 '0'，否則為 '1'
        result[i] = (msg1[i] == msg2[i]) ? '0' : '1'; 
    }
    result[len] = '\0';  // 添加結束符
}

// 發送 UDP 消息到指定客戶端 IP 和端口，返回 0 表示成功，-1 表示失敗
int send_udpmsg(const char *server_ip, int server_port, const char *client_ip, int client_port, const char *message, int source_port) {
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;

    // 創建 UDP 套接字
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1; // 失敗，返回 -1
    }

    // 設置服務器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sock_descriptor);
        return -1;
    }

    // 綁定到指定的源端口
    server_addr.sin_port = htons(source_port); // 綁定到指定的源端口

    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    // 設置目標客戶端地址
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, client_ip, &client_addr.sin_addr) <= 0) {
        perror("Invalid client IP address");
        close(sock_descriptor);
        return -1;
    }
    client_addr.sin_port = htons(client_port);

    // 發送數據包
    if (sendto(sock_descriptor, message, strlen(message), 0, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Send failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    printf("Message sent: %s\n", message);
    close(sock_descriptor);
    return 0; // 成功，返回 0
}

// 用來存儲不同端口的二進制消息
typedef struct {
    char binary_message_10000[MESSAGE_LENGTH + 1]; // 用於存儲 port 10000 的消息
    char binary_message_10001[MESSAGE_LENGTH + 1]; // 用於存儲 port 10001 的消息
} msg_data_t;

// 發送二進制數據到端口 10000 的執行緒函數
void* send_to_10000(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 服務器的 IP 地址
    const char *client_ip = "192.168.229.134"; // 客戶端的 IP 地址
    int client_port = 5001; // 這是用來接收的端口

    // 清空二進制消息以便重用
    memset(data->binary_message_10000, 0, sizeof(data->binary_message_10000));

    // 生成隨機的二進制內容消息
    genrandom_binary(data->binary_message_10000, MESSAGE_LENGTH);
    send_udpmsg(server_ip, 10000, client_ip, client_port, data->binary_message_10000, 10000);

    return NULL; // 結束這個執行緒
}

// 發送二進制數據到端口 10001 的執行緒函數
void* send_to_10001(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 服務器的 IP 地址
    const char *client_ip = "192.168.229.134"; // 客戶端的 IP 地址
    int client_port = 5001; // 這是用來接收的端口

    // 清空二進制消息以便重用
    memset(data->binary_message_10001, 0, sizeof(data->binary_message_10001));

    // 生成隨機的二進制內容消息
    genrandom_binary(data->binary_message_10001, MESSAGE_LENGTH);
    send_udpmsg(server_ip, 10001, client_ip, client_port, data->binary_message_10001, 10001);

    return NULL; // 結束這個執行緒
}

// 發送 XOR 結果到端口 10002 的執行緒函數
void* send_to_10002(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 服務器的 IP 地址
    const char *client_ip = "192.168.229.134"; // 客戶端的 IP 地址
    int client_port = 5001; // 這是用來接收的端口

    char XOR_result[MESSAGE_LENGTH + 1];

    // XOR 操作，將 10000 和 10001 的消息進行異或處理
    XOR_msg(data->binary_message_10000, data->binary_message_10001, XOR_result, MESSAGE_LENGTH);

    // 發送 XOR 結果到端口 10002
    send_udpmsg(server_ip, 10002, client_ip, client_port, XOR_result, 10002);

    return NULL;
}

int main() {
    const char *server_ip = "192.168.229.135"; // 服務器的 IP 地址
    const char *client_ip = "192.168.229.134"; // 客戶端的 IP 地址
    int client_port = 5001; // 這是用來接收的端口

    // 創建一個共享數據結構來存儲二進制消息
    msg_data_t data;

    // 發送 10 次消息
    for (int i = 0; i < 10; i++) {
        pthread_t thread_10000, thread_10001, thread_10002;

        // 創建執行緒，分別向端口 10000 和 10001 發送數據
        pthread_create(&thread_10000, NULL, send_to_10000, &data);
        pthread_create(&thread_10001, NULL, send_to_10001, &data);

        pthread_join(thread_10000, NULL);  // 等待 thread_10000 完成 
        pthread_join(thread_10001, NULL);  // 等待 thread_10001 完成 

        // 發送 XOR 結果到端口 10002
        pthread_create(&thread_10002, NULL, send_to_10002, &data);
        pthread_join(thread_10002, NULL); // 等待 thread_10002 完成

        // 可選：添加小延遲，避免網絡過載
        // sleep(1);
    }

    return 0;
}
