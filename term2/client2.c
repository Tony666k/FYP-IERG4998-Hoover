#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

typedef struct {
    int sock;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
} MPUDPPath;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 打开文件
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    // 客户端三条本地IP + 服务器三条IP
    const char* client_ips[3] = {
        "192.168.1.2",
        "192.168.3.2",
        "192.168.5.2"
    };
    const char* server_ips[3] = {
        "192.168.2.2",
        "192.168.4.2",
        "192.168.6.2"
    };

    // 统一用 5000 端口
    unsigned short srv_port = 5000;

    // 三个套接字
    MPUDPPath paths[3];
    memset(paths, 0, sizeof(paths));

    // 分别创建并绑定
    for (int i = 0; i < 3; i++) {
        // 创建 socket
        paths[i].sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (paths[i].sock < 0) {
            perror("socket");
            return 1;
        }

        // 本地地址
        memset(&paths[i].local_addr, 0, sizeof(paths[i].local_addr));
        paths[i].local_addr.sin_family = AF_INET;
        paths[i].local_addr.sin_addr.s_addr = inet_addr(client_ips[i]);
        // 为了避免端口冲突，可让它们依次 + i
        paths[i].local_addr.sin_port = htons(4000 + i);

        if (bind(paths[i].sock,
                 (struct sockaddr*)&paths[i].local_addr,
                 sizeof(paths[i].local_addr)) < 0) {
            perror("bind");
            return 1;
        }

        // 远端地址
        memset(&paths[i].remote_addr, 0, sizeof(paths[i].remote_addr));
        paths[i].remote_addr.sin_family = AF_INET;
        paths[i].remote_addr.sin_addr.s_addr = inet_addr(server_ips[i]);
        paths[i].remote_addr.sin_port = htons(srv_port);
    }

    // 读文件并轮流发送
    char buffer[BUF_SIZE];
    size_t bytes_read;
    int path_index = 0;

    while ((bytes_read = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        // 选当前路径
        MPUDPPath *p = &paths[path_index];
        // 发包
        ssize_t sent = sendto(p->sock, buffer, bytes_read, 0,
                              (struct sockaddr*)&p->remote_addr,
                              sizeof(p->remote_addr));
        if (sent < 0) {
            perror("sendto");
            break;
        }
        // 轮流切换路径: 0->1->2->0->...
        path_index = (path_index + 1) % 3;
    }

    fclose(fp);
    // 关闭所有 socket
    for (int i = 0; i < 3; i++) {
        close(paths[i].sock);
    }

    printf("File '%s' sent over 3 UDP paths.\n", argv[1]);
    return 0;
}
