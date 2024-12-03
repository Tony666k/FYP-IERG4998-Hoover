#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024       // 設置緩衝區大小
#define MESSAGE_LENGTH 8    // 定義消息長度

// 接收 UDP 消息的函數
void* receive_udpmsg(void* arg) {
    int listen_port = *((int*)arg); // 端口號
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // 創建 UDP 套接字
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return NULL; // 失敗，返回
    }

    // 設置服務器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 接受來自任意 IP 的數據包
    server_addr.sin_port = htons(listen_port); // 綁定到指定端口

    // 綁定套接字到指定端口
    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return NULL; // 失敗，返回
    }

    // 接收數據
    while (1) {
        ssize_t n = recvfrom(sock_descriptor, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Receive failed");
            close(sock_descriptor);
            return NULL; // 失敗，返回
        }

        // 顯示接收到的消息
        buffer[n] = '\0'; // 添加結束符
        printf("Received message on port %d from %s:%d -> %s\n", listen_port, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
    }

    close(sock_descriptor);
    return NULL; // 完成後返回
}

int main() {
    // 監聽端口
    int ports[] = {10001, 10000, 10002};  // 需要監聽的端口
    int num_ports = sizeof(ports) / sizeof(ports[0]);
    pthread_t threads[num_ports];  // 線程數組

    // 創建線程來監聽每個端口
    for (int i = 0; i < num_ports; i++) {
        // 創建一個線程來處理端口的接收
        if (pthread_create(&threads[i], NULL, receive_udpmsg, &ports[i]) != 0) {
            perror("Failed to create thread");
            return 1;  // 如果線程創建失敗，退出程序
        }
    }

    // 等待所有線程完成
    for (int i = 0; i < num_ports; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
