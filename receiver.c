#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5001
#define BUF_SIZE 1024

int main(){
	int sockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len;
	char buffer[BUF_SIZE];


	//UDP SOCKET
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
	perror("Socket creation failed");
	exit(EXIT_FAILURE);
	}

	//setup server_addr struct
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	//binding port
	if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
	perror("Bind failed");
	close(sockfd);
	exit(EXIT_FAILURE);
	}

	printf("Listening on port %d...\n", PORT);


	//receive the message from sender
	addr_len = sizeof(client_addr);
	while(1){
	int n = recvfrom(sockfd, (char * )buffer, BUF_SIZE, 0, (struct sockaddr*) &client_addr, &addr_len);
	buffer[n] = '\0'; //end

	printf("Received message: %s\n", buffer);
	printf("From IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	}

	close(sockfd);
	return 0;
}
