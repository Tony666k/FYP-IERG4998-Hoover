#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h> //using more samller time

#define PORT 5001
#define BUF_SIZE 1024
#define MESSAGE_LENGTH 8

unsigned int get_microsecond_seed() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;  // 返回微秒级时间戳
}

void generate_random_binary_message(char *bmessage, int length) {
    for (int i = 0; i < length; i++) {
        bmessage[i] = rand() % 2 ? '1' : '0';
    }
    bmessage[length] = '\0';
}

void send_udp_message(const char *source_ip,int source_port,const char *target_ip,int target_port){
int sockfd;
srand(get_microsecond_seed());
struct sockaddr_in server_addr,target_addr;
char message[MESSAGE_LENGTH + 1] ;
generate_random_binary_message(message, MESSAGE_LENGTH);
char message_with_uid[MESSAGE_LENGTH + 30];
int uid = rand();
snprintf(message_with_uid, sizeof(message_with_uid), "UID%d:%s", uid, message);

    
//create socket
if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
perror("Socket creation failed");
exit(EXIT_FAILURE);//exit(1);
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
sendto(sockfd,(const char *)message_with_uid,strlen(message_with_uid),0,(const struct sockaddr *)&target_addr,sizeof(target_addr));
printf("Message sent from %s:%d to %s:%d UID:%d\n",source_ip,source_port,target_ip,target_port,uid);

close(sockfd);
}

int main(){
 const char *target_ip="192.168.229.134";
 const char *source_ip="192.168.229.135";
 int source_ports[]={10000,10001,10002};
 //10000 for 1,3,5,7,9;
 //10001 for 2,4,6,8,10;
 //10002 for 1 XOR 2......9 XOR 10;
 int target_port=PORT;
 int num_ports=sizeof(source_ports)/sizeof(source_ports[0]);
 srand(time(0));//set random seed
 
//loop for send data
for (int i=1; i<=10; i++){
if( i % 2 != 0)
send_udp_message(source_ip,10000,target_ip,target_port);
if( i % 2 == 0)
send_udp_message(source_ip,10001,target_ip,target_port);
//sleep(1);//send 1 time per second
}

return 0;}

