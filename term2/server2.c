#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024

// 线程参数结构体
typedef struct {
    int sock;
    FILE *fp;
    struct sockaddr_in addr;
    char filename[64];
} ThreadArg;

// 每个线程：绑定在不同 IP:5000，持续接收数据并写入各自文件
void* recv_thread(void* arg)
{
    ThreadArg *targ = (ThreadArg*)arg;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // 在此线程内 bind 到特定 IP
    if (bind(targ->sock, (struct sockaddr*)&targ->addr, sizeof(targ->addr)) < 0) {
        perror("bind");
        return NULL;
    }
    printf("Listening on %s:%d, output -> %s\n",
           inet_ntoa(targ->addr.sin_addr),
           ntohs(targ->addr.sin_port),
           targ->filename);

    while (1) {
        ssize_t recv_len = recvfrom(targ->sock, buffer, BUF_SIZE, 0,
                                    (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            break;
        }
        // 打印接收到的数据和来源
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
        unsigned short cport = ntohs(client_addr.sin_port);
        printf("Received %zd bytes from %s:%u\n", recv_len, addr_str, cport);

        // 写入文件
        fwrite(buffer, 1, recv_len, targ->fp);
        fflush(targ->fp);
    }

    fclose(targ->fp);
    close(targ->sock);
    free(targ);
    return NULL;
}

int main()
{
    // 三个服务器 IP
    const char* server_ips[3] = {
        "192.168.2.2",
        "192.168.4.2",
        "192.168.6.2"
    };
    unsigned short port = 5000;

    // 创建三个线程
    for (int i = 0; i < 3; i++) {
        // 创建 socket
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("socket");
            return 1;
        }

        // 设置地址
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(server_ips[i]);
        addr.sin_port = htons(port);

        // 打开对应的输出文件
        char filename[64];
        snprintf(filename, sizeof(filename), "recv_path%d.bin", i);

        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            perror("fopen");
            close(sock);
            return 1;
        }

        // 分配线程参数
        ThreadArg *targ = (ThreadArg*)malloc(sizeof(ThreadArg));
        targ->sock = sock;
        targ->fp   = fp;
        targ->addr = addr;
        strcpy(targ->filename, filename);

        // 创建接收线程
        pthread_t tid;
        if (pthread_create(&tid, NULL, recv_thread, targ) != 0) {
            perror("pthread_create");
            fclose(fp);
            close(sock);
            free(targ);
            return 1;
        }
        // 分离线程，不用 join
        pthread_detach(tid);
    }

    // 主线程啥也不做，防止退出
    while (1) {
        sleep(60);
    }

    return 0;
}
