#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUF_SIZE 1024
#define PORT1 10001
#define PORT2 10002
#define PORT3 10000

int main() {
    int sockfd1, sockfd2, sockfd3;
    struct sockaddr_in server_addr1, server_addr2, server_addr3;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    char buffer[BUF_SIZE];

    // 創建三個UDP套接字
    if ((sockfd1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed for port 10001");
        exit(EXIT_FAILURE);
    }
    if ((sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed for port 10002");
        exit(EXIT_FAILURE);
    }
    if ((sockfd3 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed for port 10000");
        exit(EXIT_FAILURE);
    }

    // 設置 server_addr 結構並綁定端口
    memset(&server_addr1, 0, sizeof(server_addr1));
    server_addr1.sin_family = AF_INET;
    server_addr1.sin_addr.s_addr = INADDR_ANY;
    server_addr1.sin_port = htons(PORT1);

    memset(&server_addr2, 0, sizeof(server_addr2));
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = INADDR_ANY;
    server_addr2.sin_port = htons(PORT2);

    memset(&server_addr3, 0, sizeof(server_addr3));
    server_addr3.sin_family = AF_INET;
    server_addr3.sin_addr.s_addr = INADDR_ANY;
    server_addr3.sin_port = htons(PORT3);

    // 綁定端口
    if (bind(sockfd1, (const struct sockaddr *)&server_addr1, sizeof(server_addr1)) < 0) {
        perror("Bind failed for port 10001");
        close(sockfd1);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd2, (const struct sockaddr *)&server_addr2, sizeof(server_addr2)) < 0) {
        perror("Bind failed for port 10002");
        close(sockfd2);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd3, (const struct sockaddr *)&server_addr3, sizeof(server_addr3)) < 0) {
        perror("Bind failed for port 10000");
        close(sockfd3);
        exit(EXIT_FAILURE);
    }

    printf("Listening on ports 10001, 10002, and 10000...\n");

    // 使用select來監聽多個端口
    fd_set readfds;
    int max_fd = (sockfd1 > sockfd2 ? sockfd1 : sockfd2);
    max_fd = (max_fd > sockfd3 ? max_fd : sockfd3);

    addr_len = sizeof(client_addr);
    while (1) {
        // 清空readfds集合
        FD_ZERO(&readfds);
        FD_SET(sockfd1, &readfds);
        FD_SET(sockfd2, &readfds);
        FD_SET(sockfd3, &readfds);

        // 使用select來檢查是否有資料到來
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // 檢查從 port 10001 收到的資料
        if (FD_ISSET(sockfd1, &readfds)) {
            int n = recvfrom(sockfd1, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            buffer[n] = '\0'; // 確保資料結尾
            printf("Received message on port 10001: %s\n", buffer);
            printf("From IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // 檢查從 port 10002 收到的資料
        if (FD_ISSET(sockfd2, &readfds)) {
            int n = recvfrom(sockfd2, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            buffer[n] = '\0'; // 確保資料結尾
            printf("Received message on port 10002: %s\n", buffer);
            printf("From IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // 檢查從 port 10000 收到的資料
        if (FD_ISSET(sockfd3, &readfds)) {
            int n = recvfrom(sockfd3, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            buffer[n] = '\0'; // 確保資料結尾
            printf("Received message on port 10000: %s\n", buffer);
            printf("From IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
    }

    // 關閉套接字
    close(sockfd1);
    close(sockfd2);
    close(sockfd3);

    return 0;
}
