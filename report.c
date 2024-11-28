#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define PORT 5001 //set the udp port
#define BUF_SIZE 1024 //set the buffer size 
#define MESSAGE_LENGTH 8 //define the msg length

//Generate a random binary message of specified length
void genrandom_binary(char *msg, int len) {
    for (int i = 0; i < len; i++)
        msg[i] = rand() % 2 ? '1' : '0'; // Randomly select ‘1’ or ‘0’ to form a message string
    msg[len] = '\0'; // Add null terminator
}

// Use the XOR of MPUDP we designed to operate on binary messages
void XOR_msg(const char *msg1, const char *msg2, char *result, int len) {
    for (int i = 0; i < len; i++) {
        // Apply XOR algorithm logic - the same character outputs '0', and different characters output '1'
        result[i] = (msg1[i] == msg2[i]) ? '0' : '1'; 
    }
    result[len] = '\0'; // Add null terminator
}

// Send UDP message to the client IP and port, return 0 if send is successful, -1 if failed
int send_udpmsg(const char *server_ip, int server_port, const char *client_ip, int client_port, const char *message) {
    int sock_descriptor;
    struct sockaddr_in server_addr, client_addr;

    // Create socket
    if ((sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1; //failed, return -1
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sock_descriptor);
        return -1;
    }
    server_addr.sin_port = htons(server_port);

    // Bind the server port
    if (bind(sock_descriptor, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_descriptor);
        return -1; //failed, return -1
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
        return -1; //failed, return -1
    }

    printf("Message sent: %s\n", message);
    close(sock_descriptor);
    return 0; //success, return 0
}

// hold the binary messages for different port
typedef struct {
    char binary_message_10000[MESSAGE_LENGTH + 1]; //store the msg port 10000
    char binary_message_10001[MESSAGE_LENGTH + 1]; //store the msg port 10001
} msg_data_t;

// Thread function for sending binary data to port 10000
void* send_to_10000(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; //our server IP addr
    const char *client_ip = "192.168.229.134"; //our client IP addr
    int client_port = PORT;

    // Clear binary message for reuse
    memset(data->binary_message_10000, 0, sizeof(data->binary_message_10000));

    send_udpmsg(server_ip, 10000, client_ip, client_port, data->binary_message_10000);

    return NULL; //the end of the thread
}

// Thread function for sending binary data to port 10001
void* send_to_10001(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; //our server IP addr
    const char *client_ip = "192.168.229.134"; //our client IP addr
    int client_port = PORT;

    // Clear binary message for reuse
    memset(data->binary_message_10001, 0, sizeof(data->binary_message_10001));

    send_udpmsg(server_ip, 10001, client_ip, client_port, data->binary_message_10001);

    return NULL; //the end of the thread
}

// Thread function for sending XOR result to port 10002
void* send_to_10002(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *server_ip = "192.168.229.135"; //our server IP addr
    const char *client_ip = "192.168.229.134"; //our client IP addr
    int client_port = PORT;

    char XOR_result[MESSAGE_LENGTH + 1];

    // XOR the binary parts (without UID)
    XOR_msg(data->binary_message_10000, data->binary_message_10001, XOR_result, MESSAGE_LENGTH);

    // Send XOR result to port 10002
    send_udpmsg(server_ip, 10002, client_ip, client_port, XOR_result);

    return NULL;
}

int main() {
    const char *server_ip = "192.168.229.135"; //our server IP addr
    const char *client_ip = "192.168.229.134"; //our client IP addr
    int client_port = PORT;

    // Create a shared data structure to save the binary msg
    msg_data_t data;

    // send 10 msgs for 10 times
    for (int i = 0; i < 10; i++) {
        pthread_t thread_10000, thread_10001, thread_10002;

        // Create threads for sending data to port 10000, 10001
        pthread_create(&thread_10000, NULL, send_to_10000, &data);
        pthread_create(&thread_10001, NULL, send_to_10001, &data);

        pthread_join(thread_10000, NULL);  // Wait for thread_10000 to complete 
        pthread_join(thread_10001, NULL);  // Wait for thread_10001 to complete 

        // send the XOR result to 10002
        pthread_create(&thread_10002, NULL, send_to_10002, &data);
        pthread_join(thread_10002, NULL); // Wait for thread_10002 to complete

        // Optional: Add a small delay
        // sleep(1);
    }

    return 0;
}
