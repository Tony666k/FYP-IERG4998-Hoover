#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5001 // Set the UDP port (for the client)
#define BUF_SIZE 1024 // Set the buffer size
#define MESSAGE_LENGTH 4 // Define the message length (4 bits)

// Define a constant message with 4 '1's
#define FIXED_MESSAGE "1111"

// Send UDP message to the client IP and port, return 0 if send is successful, -1 if failed
int send_udpmsg(const char *server_ip, int server_port, const char *client_ip, int client_port, const char *message) {
    int sock_descriptor;
    struct sockaddr_in client_addr;

    // Create socket
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1; // Failed, return -1
    }

    // Set destination address
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, client_ip, &client_addr.sin_addr) <= 0) {
        perror("Invalid client IP address");
        close(sock_descriptor);
        return -1;
    }
    client_addr.sin_port = htons(client_port);

    // Send the data packet
    if (sendto(sock_descriptor, message, strlen(message), 0, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Send failed");
        close(sock_descriptor);
        return -1; // Failed, return -1
    }

    printf("Message sent to port %d: %s\n", server_port, message);
    close(sock_descriptor);
    return 0; // Success, return 0
}

// Thread function for sending binary data to port 10000
void* send_to_10000(void *arg) {
    const char *server_ip = "192.168.229.135"; // Our server IP addr
    const char *client_ip = "192.168.229.134"; // Our client IP addr
    int client_port = 10000;

    // Send the fixed message to port 10000 once
    send_udpmsg(server_ip, 10000, client_ip, client_port, FIXED_MESSAGE);

    return NULL; // End of the thread
}

// Thread function for sending binary data to port 10001
void* send_to_10001(void *arg) {
    const char *server_ip = "192.168.229.135"; // Our server IP addr
    const char *client_ip = "192.168.229.134"; // Our client IP addr
    int client_port = 10001;

    // Send the fixed message to port 10001 once
    send_udpmsg(server_ip, 10001, client_ip, client_port, FIXED_MESSAGE);

    return NULL; // End of the thread
}

int main() {
    const char *server_ip = "192.168.229.135"; // Our server IP addr
    const char *client_ip = "192.168.229.134"; // Our client IP addr
    int client_port = PORT;

    // Create threads for sending data to port 10000 and 10001
    pthread_t thread_10000, thread_10001;

    // Loop to send data twice
    for (int i = 0; i < 2; i++) {
        // Start sending data to port 10000 and 10001
        pthread_create(&thread_10000, NULL, send_to_10000, NULL);
        pthread_create(&thread_10001, NULL, send_to_10001, NULL);

        // Wait for both threads to complete
        pthread_join(thread_10000, NULL);
        pthread_join(thread_10001, NULL);

        // Optional: Add a small delay to avoid overwhelming the network (use sleep if needed)
        // sleep(1);
    }

    return 0;
}
