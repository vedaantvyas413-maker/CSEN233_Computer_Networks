#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

//Declare socket descriptor.
int sockfd;

//Define the number of clients/threads that can be served
#define N 100
int threadCount = 0;
pthread_t clients[N]; //declaring N threads

//Declare server address to which to bind for receiving messages and client address to fill in sending address
struct sockaddr_in servAddr, clienAddr;

typedef struct {
   int connfd;
   struct sockaddr_in clienAddr;
} client_args_t;

//Connection handler for servicing client requests for file transfer
void *connectionHandler(void *arg){
   client_args_t *client = (client_args_t *)arg;
   int connfd = client->connfd;
   struct sockaddr_in clienAddr = client->clienAddr;
   char rbuf[256], sbuf[256];

   //Connection established, server begins to read and write to the connecting client
   printf("Connection Established with client IP: %s and Port: %d. This is the threaded server.\n", inet_ntoa(clienAddr.sin_addr), ntohs(clienAddr.sin_port));
   //receive name of the file from the client
   int rb = recv(connfd, rbuf, sizeof(rbuf), 0);
   if (rb <= 0) {
       perror("Failed to receive file name");
       close(connfd);
       free(client);
       return NULL;
   }
   if (rb >= (int)sizeof(rbuf)) {
       rbuf[sizeof(rbuf) - 1] = '\0';
   } else {
       rbuf[rb] = '\0';
   }
   while (rb > 0 && (rbuf[rb - 1] == '\n' || rbuf[rb - 1] == '\r')) {
       rbuf[rb - 1] = '\0';
       rb--;
   }
   printf("Received file name: %s\n", rbuf);

   //open file and send to client
   FILE *fp = fopen(rbuf, "rb");
   if (fp == NULL) {
       perror("File open error");
       close(connfd);
       free(client);
       return NULL;
   }

   //read file and send to connection descriptor
   int n;
   while ((n = fread(sbuf, sizeof(char), sizeof(sbuf), fp)) > 0) {
       int total_sent = 0;
       while (total_sent < n) {
           int sent = send(connfd, sbuf + total_sent, n - total_sent, 0);
           if (sent < 0) {
               perror("Failed to send file data");
               fclose(fp);
               close(connfd);
               free(client);
               return NULL;
           }
           total_sent += sent;
       }
   }

   printf("File transfer complete\n");

   //close file
   fclose(fp);

   //Close connection descriptor
   close(connfd);
   free(client);
   return NULL;
}

int main(int argc, char *argv[]){
 //Get from the command line, server IP, src and dst files.
 if (argc != 2){
	printf ("Usage: %s <port #> \n",argv[0]);
	exit(0);
 } 
 //Open a TCP socket, if successful, returns a descriptor
 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Failure to setup an endpoint socket");
    exit(1);
 }

 //Setup the server address to bind using socket addressing structure
 memset(&servAddr, 0, sizeof(servAddr));
 servAddr.sin_family = AF_INET;
 servAddr.sin_port = htons(atoi(argv[1]));
 servAddr.sin_addr.s_addr = INADDR_ANY;

 //bind IP address and port for server endpoint socket
 if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
     perror("Failure to bind server address to endpoint socket");
     exit(1);
 }

 // Server listening to the socket endpoint, and can queue 5 client requests
 listen(sockfd, 5);
 socklen_t sin_size = sizeof(struct sockaddr_in);
 printf("Server waiting for client at port %d\n", atoi(argv[1]));
 
 while (1){
    if (threadCount >= N){
        perror("Too many connections");
        break;
    }

   int connfd;
   sin_size = sizeof(clienAddr);
   if ((connfd = accept(sockfd, (struct sockaddr*)&clienAddr, &sin_size)) < 0) {
       perror("Failure to accept connection to the client");
       continue;
   }
   client_args_t *client = (client_args_t *)malloc(sizeof(client_args_t));
   if (client == NULL) {
       perror("Failed to allocate client args");
       close(connfd);
       continue;
   }
    // pass correct connfd argument'
    client->connfd = connfd;
    client->clienAddr = clienAddr;

    if(pthread_create(&clients[threadCount], NULL, connectionHandler, (void*) client) != 0){
      perror("Unable to create a thread");
      close(connfd);
      free(client);
      continue;
   }
   else 
      printf("Thread %d has been created to service client request\n",++threadCount);
 }
 for(int i = 0; i < threadCount; i++){
      pthread_join(clients[i], NULL);
 }
 return 0;
}
