#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h> // using smaller time

#define PORT 5001
#define BUF_SIZE 1024
#define MESSAGE_LENGTH 8

// Get a random UID using microsecond timestamp
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
void message_combine(char *message_binary, int *uid) {
    *uid = UID(); // Generate a random UID
    snprintf(message_binary, BUF_SIZE, "UID(%d):%s", *uid, message_binary); // Combine UID and binary message
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

int main() {
    const char *target_ip = "192.168.229.134";
    const char *source_ip = "192.168.229.135";
    int source_ports[] = {10000, 10001, 10002};
    int target_port = PORT;
    int num_ports = sizeof(source_ports) / sizeof(source_ports[0]);
    srand(time(0)); // Set random seed
    char binary_10000[BUF_SIZE], binary_10001[BUF_SIZE];
    char message_send_10000[BUF_SIZE], message_send_10001[BUF_SIZE];
    char xor_result[MESSAGE_LENGTH + 1]; // Store the XOR result

    int uid_10000, uid_10001;

    // Loop for sending data
    for (int i = 1; i <= MESSAGE_LENGTH; i++) {
        if (i % 2 != 0) {
            // Generate message for port 10000
            message_combine(message_send_10000, &uid_10000);

            // Try to send message for port 10000 (but don't check result)
            send_udp_message(source_ip, 10000, target_ip, target_port, message_send_10000);
        }

        if (i % 2 == 0) {
            // Generate message for port 10001
            message_combine(message_send_10001, &uid_10001);

            // Try to send message for port 10001 (but don't check result)
            send_udp_message(source_ip, 10001, target_ip, target_port, message_send_10001);

            // XOR the messages from 10000 and 10001
            xor_messages(message_send_10000, message_send_10001, xor_result, MESSAGE_LENGTH);

            // Send XOR result to port 10002 (this happens every loop)
            send_udp_message(source_ip, 10002, target_ip, target_port, xor_result);
            printf("Sent XOR result to port 10002: %s\n", xor_result);
        }
    }

    return 0;
}
