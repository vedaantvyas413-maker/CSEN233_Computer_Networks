//TCP Client
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    //Get from the command line, server IP,and port # 
    if (argc != 3){
		printf ("Usage: %s <ip of server> <port #> \n",argv[0]);
		exit(0);
    } 
    //Declare socket file descriptor and buffer
    int sockfd;
    char rbuf[256], sbuf[256];

    //Declare server address to connect to
    struct sockaddr_in servAddr;
    struct hostent *host;

    //get hostname
    host = (struct hostent *) gethostbyname(argv[1]);

    //Open a socket, if successful, returns
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Failure to setup an endpoint socket");
        exit(0);
    }

    //Set the server address to send using socket addressing structure
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2]));
    servAddr.sin_addr = *((struct in_addr *)host->h_addr);

    //Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr))){
        perror("Failure to connect to the server");
        exit(1);
    }
    //Ask for filename, write only the length of the filename read



    printf("Client: Enter the filename to request from server: ");
    if (fgets(sbuf, sizeof(sbuf), stdin) == NULL) {
        perror("Failed to read filename");
        close(sockfd);
        exit(1);
    }
    size_t len = strlen(sbuf);
    while (len > 0 && (sbuf[len - 1] == '\n' || sbuf[len - 1] == '\r')) {
        sbuf[len - 1] = '\0';
        len--;
    }
    if (len == 0) {
        fprintf(stderr, "Empty filename\n");
        close(sockfd);
        exit(1);
    }
    write(sockfd, sbuf, len);



    //Client reads file from server
    char outname[512];
    snprintf(outname, sizeof(outname), "received_%s", sbuf);
    FILE *fp = fopen(outname, "wb");
    if (fp == NULL) {
        perror("Failed to open file");
        close(sockfd);
        exit(1);
    }
    size_t total_bytes = 0;
    while(1){
        int n = read(sockfd, rbuf, sizeof(rbuf));
        if (n < 0) {
            perror("Failed to read from server");
            break;
        }
        if (n == 0) {
            break;
        }
        fwrite(rbuf, sizeof(char), n, fp);
        total_bytes += (size_t)n;
    }
    fclose(fp);
    printf("Download complete: %zu bytes received\n", total_bytes);
    //Close socket descriptor
    close(sockfd);
    return 0;
}
