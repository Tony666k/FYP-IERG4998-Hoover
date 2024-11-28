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
void xor_msg(const char *msg1, const char *msg2, char *result, int len) {
    for (int i = 0; i < len; i++) {
        // Apply XOR algorithm logic - the same character outputs '0', and different characters output '1'
        result[i] = (msg1[i] == msg2[i]) ? '0' : '1'; 
    }
    result[len] = '\0'; // Add null terminator
}

// Send UDP message to the target IP and port, return 0 if send is successful, -1 if failed
int send_udpmsg(const char *source_ip, int source_port, const char *target_ip, int target_port, const char *message) {
    int sockfd;
    struct sockaddr_in server_addr, target_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;//failed,return -1
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
        return -1;//failed,return -1
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
        return -1;//failed,return -1
    }

    printf("Message sent: %s\n", message);
    close(sockfd);
    return 0; //success,return 0
}

// hold the binary messages for different port
typedef struct {
    char binary_message_10000[MESSAGE_LENGTH + 1];//store the msg port 10000
    char binary_message_10001[MESSAGE_LENGTH + 1];//store the msg port 10001
} msg_data_t;

// Thread function for sending binary data to port 10000
void* send_to_10000(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *source_ip = "192.168.229.135";//our server IP addr
    const char *target_ip = "192.168.229.134";//our client IP addr
    int target_port = PORT;

    // Clear binary message for reuse
    memset(data->binary_message_10000, 0, sizeof(data->binary_message_10000));

    // Generate random binary content msg
    genrandom_binary(data->binary_message_10000, MESSAGE_LENGTH);
    send_udpmsg(source_ip, 10000, target_ip, target_port, data->binary_message_10000);

    return NULL;//the end of the thread
}

// Thread function for sending binary data to port 10001
void* send_to_10001(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *source_ip = "192.168.229.135";//our server IP addr
    const char *target_ip = "192.168.229.134";//our client IP addr
    int target_port = PORT;

      // Clear binary message for reuse
    memset(data->binary_message_10001, 0, sizeof(data->binary_message_10001));

    // Generate random binary content msg
    genrandom_binary(data->binary_message_10001, MESSAGE_LENGTH);
    send_udpmsg(source_ip, 10001, target_ip, target_port, data->binary_message_10001);

    return NULL;//the end of the thread
}

// Thread function for sending XOR result to port 10002
void* send_to_10002(void *arg) {
    msg_data_t *data = (msg_data_t *)arg;
    const char *source_ip = "192.168.229.135";//our server IP addr
    const char *target_ip = "192.168.229.134";//our client IP addr
    int target_port = PORT;

    char xor_result[MESSAGE_LENGTH + 1];

    // XOR the binary parts (without UID)
    xor_msg(data->binary_message_10000, data->binary_message_10001, xor_result, MESSAGE_LENGTH);

    // Send XOR result to port 10002
    send_udpmsg(source_ip, 10002, target_ip, target_port, xor_result);

    return NULL;
}

int main() {
    const char *source_ip = "192.168.229.135";//our server IP addr
    const char *target_ip = "192.168.229.134";//our client IP addr
    int target_port = PORT;

    // Create a shared data structure to save the binary msg
    msg_data_t data;

    // send 10 msgs for 10 times
    for (int i = 0; i < 10; i++) {
        pthread_t thread1, thread2, thread3;

        // Create threads for sending data to port 10000, 10001
        pthread_create(&thread1, NULL, send_to_10000, &data);
        pthread_create(&thread2, NULL, send_to_10001, &data);

         
        pthread_join(thread1, NULL);  // Wait for thread1 to complete 
        pthread_join(thread2, NULL);  // Wait for thread2 to complete 

        // send the XOR result to 10002
        pthread_create(&thread3, NULL, send_to_10002, &data);
        pthread_join(thread3, NULL); // Wait for thread3 to complete 
 
        // Optional: Add a small delay
        // sleep(1);
    }

    return 0;
}
