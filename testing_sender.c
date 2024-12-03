#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define PORT 5001 // 不需要再用 5001 這個端口，僅用來綁定發送者的 socket
#define BUF_SIZE 1024 // 設定緩衝區大小
#define MESSAGE_LENGTH 8 // 設定消息長度

// 生成隨機的二進制消息
void genrandom_binary(char *msg, int len) {
    for (int i = 0; i < len; i++)
        msg[i] = rand() % 2 ? '1' : '0'; // 隨機選擇 '1' 或 '0' 生成消息
    msg[len] = '\0'; // 添加終止符
}

// 進行 XOR 運算
void XOR_msg(const char *msg1, const char *msg2, char *result, int len) {
    for (int i = 0; i < len; i++) {
        // XOR 邏輯：相同字符為 '0'，不同字符為 '1'
        result[i] = (msg1[i] == msg2[i]) ? '0' : '1'; 
    }
    result[len] = '\0'; // 添加終止符
}

// 發送 UDP 消息到指定的 client
int send_udpmsg(const char *server_ip, int server_port, const char *client_ip, int client_port, const char *message) {
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;

    // 創建 socket
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
    server_addr.sin_port = htons(server_port);

    // 綁定服務器端口
    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    // 設置客戶端地址
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, client_ip, &client_addr.sin_addr) <= 0) {
        perror("Invalid client IP address");
        close(sock_descriptor);
        return -1;
    }
    client_addr.sin_port = htons(client_port);

    // 發送消息
    if (sendto(sock_descriptor, message, strlen(message), 0, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Send failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    printf("Message sent: %s\n", message);
    close(sock_descriptor);
    return 0; // 成功，返回 0
}

// 持有不同端口消息的結構體
typedef struct {
    char binary_message_10000[MESSAGE_LENGTH + 1]; // 用於存儲端口 10000 的消息
    char binary_message_10001[MESSAGE_LENGTH + 1]; // 用於存儲端口 10001 的消息
} msg_data_t;

// 向 10000 端口發送二進制數據的線程
void* send_to_10000(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 發送者 IP
    const char *client_ip = "192.168.229.134"; // 接收者 IP
    int client_port = 10000; // 目標端口 10000

    // 清空消息
    memset(data->binary_message_10000, 0, sizeof(data->binary_message_10000));

    // 生成隨機二進制消息
    genrandom_binary(data->binary_message_10000, MESSAGE_LENGTH);
    send_udpmsg(server_ip, 10000, client_ip, client_port, data->binary_message_10000); // 發送消息

    return NULL;
}

// 向 10001 端口發送二進制數據的線程
void* send_to_10001(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 發送者 IP
    const char *client_ip = "192.168.229.134"; // 接收者 IP
    int client_port = 10001; // 目標端口 10001

    // 清空消息
    memset(data->binary_message_10001, 0, sizeof(data->binary_message_10001));

    // 生成隨機二進制消息
    genrandom_binary(data->binary_message_10001, MESSAGE_LENGTH);
    send_udpmsg(server_ip, 10001, client_ip, client_port, data->binary_message_10001); // 發送消息

    return NULL;
}

// 向 10002 端口發送 XOR 計算結果的線程
void* send_to_10002(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; // 發送者 IP
    const char *client_ip = "192.168.229.134"; // 接收者 IP
    int client_port = 10002; // 目標端口 10002

    char XOR_result[MESSAGE_LENGTH + 1];

    // 計算 XOR 結果
    XOR_msg(data->binary_message_10000, data->binary_message_10001, XOR_result, MESSAGE_LENGTH);

    // 發送 XOR 計算結果
    send_udpmsg(server_ip, 10002, client_ip, client_port, XOR_result);

    return NULL;
}

int main() {
    const char *server_ip = "192.168.229.135"; // 發送者 IP
    const char *client_ip = "192.168.229.134"; // 接收者 IP
    int client_port = PORT; // 客戶端端口，這裡不使用，因為發送端口不同

    // 創建共享數據結構來存儲消息
    msg_data_t data;

    // 發送 10 次消息
    for (int i = 0; i < 10; i++) {
        pthread_t thread_10000, thread_10001, thread_10002;

        // 創建線程來向 10000, 10001 發送數據
        pthread_create(&thread_10000, NULL, send_to_10000, &data);
        pthread_create(&thread_10001, NULL, send_to_10001, &data);

        // 等待線程完成
        pthread_join(thread_10000, NULL);  
        pthread_join(thread_10001, NULL);  

        // 發送 XOR 計算結果到 10002
        pthread_create(&thread_10002, NULL, send_to_10002, &data);
        pthread_join(thread_10002, NULL); // 等待線程完成

        // 可選：添加小延遲（視情況而定）
        // sleep(1);
    }

    return 0;
}
