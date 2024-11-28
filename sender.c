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

// Generate random binary message
void Binary_choose(char *message, int length) {
    for (int i = 0; i < length; i++)
        message[i] = rand() % 2 ? '1' : '0'; // Randomly choose '1' or '0'
    message[length] = '\0'; // Add null terminator
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

// Structure to hold the binary messages for 10000 and 10001
typedef struct {
    char binary_message_10000[MESSAGE_LENGTH + 1];
    char binary_message_10001[MESSAGE_LENGTH + 1];
} message_data_t;

// Thread function for sending data to port 10000
void* send_to_10000(void *arg) {
    message_data_t *data = (message_data_t *)arg;
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    // Clear binary message before each use
    memset(data->binary_message_10000, 0, sizeof(data->binary_message_10000));

    // Generate the message with binary content
    Binary_choose(data->binary_message_10000, MESSAGE_LENGTH);
    send_udp_message(source_ip, 10000, target_ip, target_port, data->binary_message_10000);

    return NULL;
}

// Thread function for sending data to port 10001
void* send_to_10001(void *arg) {
    message_data_t *data = (message_data_t *)arg;
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    // Clear binary message before each use
    memset(data->binary_message_10001, 0, sizeof(data->binary_message_10001));

    // Generate the message with binary content
    Binary_choose(data->binary_message_10001, MESSAGE_LENGTH);
    send_udp_message(source_ip, 10001, target_ip, target_port, data->binary_message_10001);

    return NULL;
}

// Thread function for sending XOR result to port 10002
void* send_to_10002(void *arg) {
    message_data_t *data = (message_data_t *)arg;
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    char xor_result[MESSAGE_LENGTH + 1];

    // XOR the binary parts (without UID)
    xor_messages(data->binary_message_10000, data->binary_message_10001, xor_result, MESSAGE_LENGTH);

    // Send XOR result to port 10002
    send_udp_message(source_ip, 10002, target_ip, target_port, xor_result);

    return NULL;
}

int main() {
    const char *source_ip = "192.168.229.135";
    const char *target_ip = "192.168.229.134";
    int target_port = PORT;

    // Create a shared data structure
    message_data_t data;

    // Repeat 10 times
    for (int i = 0; i < 10; i++) {
        pthread_t thread1, thread2, thread3;

        // Create threads for sending data to port 10000, 10001, and 10002
        pthread_create(&thread1, NULL, send_to_10000, &data);
        pthread_create(&thread2, NULL, send_to_10001, &data);

        // Wait for thread1 and thread2 to complete (send messages to 10000 and 10001)
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        // After sending data to 10000 and 10001, XOR and send to 10002
        pthread_create(&thread3, NULL, send_to_10002, &data);
        pthread_join(thread3, NULL);

        // Optional: Add a sleep if you want a small delay between repetitions (e.g., 1 second)
        // sleep(1);
    }

    return 0;
}
