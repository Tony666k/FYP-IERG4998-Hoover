#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define PORT 5001
#define BUF_SIZE 1024
#define MESSAGE_LENGTH 8

// Generate random UID using microsecond timestamp
int UID() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec * 1000000 + tv.tv_usec);
    return rand();
}

// Generate a random binary message of the given length
void Binary_choose(char *message, int length) {
    for (int i = 0; i < length; i++)
        message[i] = rand() % 2 ? '1' : '0'; // Randomly choose '1' or '0'
    message[length] = '\0'; // Add null terminator
}

// Combine the binary message and UID into a string
void message_combine(char *message_combined, int *uid, char *binary_message) {
    Binary_choose(binary_message, MESSAGE_LENGTH); // Generate random binary message
    *uid = UID(); // Generate a random UID
    snprintf(message_combined, BUF_SIZE, "UID(%d):%s", *uid, binary_message); // Combine UID and binary message
}

// XOR two binary messages and return the result
void xor_messages(const char *msg1, const char *msg2, char *result, int length) {
    for (int i = 0; i < length; i++) {
        result[i] = (msg1[i] == msg2[i]) ? '0' : '1'; // XOR logic
    }
    result[length] = '\0'; // Null-terminate the result
}

// Send UDP message to the target IP and port, return 0 if send is successful, -1 if failed
int send_udp_message(const char *source_ip, int source_port, const char *target_ip, int target_port, const char *message) {
    int sockfd;
    struct sockaddr_in server_addr, target_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Set source address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(source_ip);
    server_addr.sin_port = htons(source_port);

    // Bind the source port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    // Set destination address
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(target_ip);
    target_addr.sin_port = htons(target_port);

    // Send the data packet
    if (sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
        perror("Send failed");
        close(sockfd);
        return -1;
    }

    printf("Message sent: %s\n", message);
    close(sockfd);
    return 0; // Send successful
}

// Thread function for sending data to port 10000
void* send_to_10000(void *arg) {
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    char message_send_10000[BUF_SIZE], binary_message_10000[MESSAGE_LENGTH + 1];
    int uid_10000;
    
    message_combine(message_send_10000, &uid_10000, binary_message_10000);
    send_udp_message(source_ip, 10000, target_ip, target_port, message_send_10000);
    return NULL;
}

// Thread function for sending data to port 10001
void* send_to_10001(void *arg) {
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    char message_send_10001[BUF_SIZE], binary_message_10001[MESSAGE_LENGTH + 1];
    int uid_10001;
    
    message_combine(message_send_10001, &uid_10001, binary_message_10001);
    send_udp_message(source_ip, 10001, target_ip, target_port, message_send_10001);
    return NULL;
}

int main() {
    // Create threads for sending data to port 10000 and 10001
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, send_to_10000, NULL);
    pthread_create(&thread2, NULL, send_to_10001, NULL);

    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
