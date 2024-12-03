#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024       // 設置緩衝區大小
#define MESSAGE_LENGTH 8    // 定義消息長度

// 接收 UDP 消息
int receive_udpmsg(int listen_port) {
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // 創建 UDP 套接字
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1; // 失敗，返回 -1
    }

    // 設置服務器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 接受任何來自任意 IP 的數據包
    server_addr.sin_port = htons(listen_port); // 綁定到指定的端口

    // 綁定套接字到指定的端口
    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    // 接收數據
    ssize_t n = recvfrom(sock_descriptor, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
    if (n < 0) {
        perror("Receive failed");
        close(sock_descriptor);
        return -1; // 失敗，返回 -1
    }

    // 顯示接收到的消息
    buffer[n] = '\0'; // 添加結束符
    printf("Received message from %s:%d -> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

    close(sock_descriptor);
    return 0; // 成功，返回 0
}

int main() {
    // 監聽端口
    int ports[] = {10000, 10001, 10002};  // 接收端口
    int num_ports = sizeof(ports) / sizeof(ports[0]);

    // 不斷輪詢各個端口，等待接收消息
    for (int i = 0; i < num_ports; i++) {
        printf("Listening on port %d...\n", ports[i]);
        if (receive_udpmsg(ports[i]) < 0) {
            fprintf(stderr, "Error receiving data on port %d\n", ports[i]);
            return 1; // 出現錯誤，退出程序
        }
    }

    return 0;
}
