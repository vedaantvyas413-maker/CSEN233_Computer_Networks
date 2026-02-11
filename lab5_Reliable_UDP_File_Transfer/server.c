//UDP Server - Side (with rdt3.0)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PLOSTMSG 5
typedef struct {
  int seq_ack;
  int len;
  int cksum;
} Header;
typedef struct {
   Header header;
  char data[10];
} Packet;

//getChecksum()
int getChecksum(Packet packet) {
  packet.header.cksum = 0;
  int checksum = 0;
  char *ptr = (char *)&packet;
  char *end = ptr + sizeof(Header) + packet.header.len;
  while (ptr < end) {
      checksum ^= *ptr++;
  }
  return checksum;
}

//print packet
void printPacket(Packet packet) {
   printf("Packet{ header: { seq_ack: %d, len: %d, cksum: %d }, data: \"",
   packet.header.seq_ack,
   packet.header.len,
   packet.header.cksum);
   fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
   printf("\" }\n");
}

//serverSend()
void serverSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, int seqnum) {
  // Simulating a chance that ACK gets lost
  if (rand() % PLOSTMSG == 0) {
     printf("Dropping ACK\n");
  }
  else{
     Packet packet;
     //prepare and send the ACK
     packet.header.seq_ack = seqnum;
     packet.header.len = 0;
     packet.header.cksum = getChecksum(packet);
     sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);

     printf("Sent ACK %d, checksum %d\n", packet.header.seq_ack, packet.header.cksum);
  }
}

Packet serverReceive(int sockfd, struct sockaddr *clientAddr, socklen_t *addrLen, int seqnum, int *last_ack) {
    Packet packet;
    while (1) {
        //Receive a packet from the client
        recvfrom(sockfd, &packet, sizeof(packet), 0, clientAddr, addrLen);
        // validate the length of the packet

        if (packet.header.len < 0 || packet.header.len > 10) { //invalid length
            serverSend(sockfd, clientAddr, *addrLen, *last_ack);
            printf("Invalid packet length: %d\n", packet.header.len);
            continue;
        }
     // print what was received
     printf("Received: ");
     printPacket(packet);
     //verify the checksum and the sequence number
     if (packet.header.cksum != getChecksum(packet)) {
          printf("Bad checksum, expected %d\n", getChecksum(packet));
           serverSend(sockfd, clientAddr, *addrLen, *last_ack);
          continue;
          } else if (packet.header.seq_ack != seqnum) {
               printf("Bad seqnum, expected %d\n", seqnum);
                serverSend(sockfd, clientAddr, *addrLen, *last_ack);
              continue;
          } else {
             printf("Good packet\n");
             serverSend(sockfd, clientAddr, *addrLen, seqnum);
             *last_ack = seqnum;
            break;
          }

    }
    printf("\n");
    return packet;
}

int main(int argc, char *argv[]) {
   // check arguments
   if (argc != 3) {
     fprintf(stderr, "Usage: %s <port> <outfile>\n", argv[0]);
     exit(1);
   }
  // seed the RNG
  srand((unsigned)time(NULL));
  // create a socket
   int sockfd;

   struct sockaddr_in servAddr, clientAddr;
   socklen_t addrLen = sizeof(clientAddr);
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Failure to setup an endpoint socket");
    exit(1);
}
servAddr.sin_family = AF_INET;
servAddr.sin_port = htons(atoi(argv[1])); //Port 5000 is assigned
servAddr.sin_addr.s_addr = INADDR_ANY; //Local IP address of any interface

if ((bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0){
perror("Failure to bind server address to the endpoint socket");
exit(1);
}

  // open file using argv[2]
  int fp=open(argv[2],O_CREAT | O_RDWR,0666);
  if(fp<0){
      perror("file failed to open\n");
      exit(1);
   }

// get file contents from client and save it to the file
int seqnum = 0;
int last_ack = 1;
Packet packet;
do {
    packet = serverReceive(sockfd, (struct sockaddr *)&clientAddr, &addrLen, seqnum, &last_ack);
    if (packet.header.len > 0) {
        lseek(fp, 0, SEEK_END);
        write(fp, packet.data, packet.header.len);
    }
    seqnum = (seqnum + 1) % 2;
} while (packet.header.len > 0);
 //cleanup
 close(fp);
 close(sockfd);
 return 0;
}
