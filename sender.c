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
void message_combine(char *message_combined, int *uid) {
    char binary_message[MESSAGE_LENGTH + 1];
    Binary_choose(binary_message, MESSAGE_LENGTH); // Generate random binary message
    *uid = UID(); // Generate a random UID
    snprintf(message_combined, BUF_SIZE, "UID(%d):%s", *uid, binary_message); // Combine UID and binary message
}

// Send UDP message to the target IP and port
void send_udp_message(const char *source_ip, int source_port, const char *target_ip, int target_port, const char *message) {
    int sockfd;
    struct sockaddr_in server_addr, target_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    printf("Message sent: %s\n", message);
    close(sockfd);
}

int main() {
    const char *target_ip = "192.168.229.134";
    const char *source_ip = "192.168.229.135";
    int source_ports[] = {10000, 10001, 10002};
    int target_port = PORT;
    int num_ports = sizeof(source_ports) / sizeof(source_ports[0]);
    srand(time(0)); // Set random seed

    char message_combined[BUF_SIZE];
    int uid = 0;

    // Loop for sending data
    for (int i = 1; i <= MESSAGE_LENGTH; i++) {
        if (i % 2 != 0) {
            message_combine(message_combined, &uid); // Generate message for odd cycles
            send_udp_message(source_ip, 10000, target_ip, target_port, message_combined);
        } else {
            message_combine(message_combined, &uid); // Generate message for even cycles
            send_udp_message(source_ip, 10001, target_ip, target_port, message_combined);
        }
    }

    return 0;
}
