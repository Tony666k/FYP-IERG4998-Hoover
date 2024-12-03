#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024 // 設定緩衝區大小

// 定義接收端每個端口的處理函數
void* receive_udpmsg(void* arg) {
    int listen_port = *((int*)arg); // 端口號
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    printf("Starting to listen on port %d...\n", listen_port); // 調試輸出

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
    pthread_t thread_10000, thread_10001, thread_10002;
    int port_10000 = 10000;
    int port_10001 = 10001;
    int port_10002 = 10002;

    // 創建處理三個端口的線程
    pthread_create(&thread_10000, NULL, receive_udpmsg, &port_10000);
    pthread_create(&thread_10001, NULL, receive_udpmsg, &port_10001);
    pthread_create(&thread_10002, NULL, receive_udpmsg, &port_10002);

    // 等待線程結束
    pthread_join(thread_10000, NULL);
    pthread_join(thread_10001, NULL);
    pthread_join(thread_10002, NULL);

    return 0;
}
