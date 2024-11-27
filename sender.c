#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5001
#define BUF_SIZE 1024

void send_udp_message(const char *source_ip,int source_port,const char *target_ip,int target_port){
int sockfd;
struct sockaddr_in server_addr,target_addr;
char message[]="MPUDP Test Message";

//create socket
if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
perror("Socket creation failed");
exit(EXIT_FAILURE);
}

//set source addr
memset(&server_addr,0,sizeof(server_addr));
server_addr.sin_family=AF_INET;
server_addr.sin_addr.s_addr=inet_addr(source_ip);
server_addr.sin_port=htons(source_port);

//bind the source port
if(bind(sockfd,(const struct sockaddr *)&server_addr,sizeof(server_addr))<0){
perror("Bind failed");
close(sockfd);
exit(EXIT_FAILURE);}

//set the destination addr
memset(&target_addr,0,sizeof(target_addr));
target_addr.sin_family=AF_INET;
target_addr.sin_addr.s_addr=inet_addr(target_ip);
target_addr.sin_port=htons(target_port);

//send the data packet
sendto(sockfd,(const char *)message,strlen(message),0,(const struct sockaddr *)&target_addr,sizeof(target_addr));
printf("Message sent from %s:%d to %s:%d\n",source_ip,source_port,target_ip,target_port);
close(sockfd);
}

int main(){
 const char *target_ip="192.168.84.129";
 const char *source_ip="192.168.84.130";
 int source_ports[]={10000,10001};
 int target_port=PORT;
 int num_ports=sizeof(source_ports)/sizeof(source_ports[0]);
 srand(time(0));//set random seed
 
//loop for send data
while(1){
int source_port =source_ports[rand()%num_ports];
send_udp_message(source_ip,source_port,target_ip,target_port);
sleep(1);//send 1 time per second
}

return 0;}

