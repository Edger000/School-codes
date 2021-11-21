#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void connectToServer(int, struct sockaddr_in);
int sendEmail(int);
void handleServer(int);
int main(){

    char *ip = "192.168.56.103";
    /*use 110 for POP standard port list*/
    int port = 110; 
    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;  
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
      perror("[-]Socket error");
      exit(1);
    }   
    printf("[+]TCP server socket created.\n");  
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);   
    connectToServer(sock, addr);

    if (sendEmail(sock) < 0){
      close(sock);
      return -1;
    }

    handleServer(sock);
    close(sock);
    printf("Disconnected from the server.\n");  
    return 0;

}
/*This function will combine the input of the email into one string and then send it to the server*/
void handleSend(int sock){
    printf("Enter email to send: ");
    char emailToSend[1024];
    memset(emailToSend, 0, 1024);
    char emailAdd[100];
    memset(emailAdd, 0, 100);
    fgets(emailAdd, 100, stdin);
    printf("Enter email header: ");
    char emailHead[100];
    memset(emailHead, 0, 100);
    fgets(emailHead, 100, stdin);
    printf("Enter email description: ");
    char emailDesc[840];
    memset(emailDesc, 0, 840);
    fgets(emailDesc, 100, stdin);
    
    char discrim[] = "|";
    strcat(emailAdd, discrim);
    strcat(emailToSend, emailAdd);
    strcat(emailHead, discrim);
    strcat(emailToSend, emailHead);
    strcat(emailToSend, emailDesc);
    send(sock, emailToSend, strlen(emailToSend), 0);
    printf("Sending email to %s", emailAdd);
}
/*Used to seperate the buffer recieved from the server that is emailfrom emmailheader emailDesc*/
void handleReceive(int sock){
    char buffer[1024];
    memset(buffer, 0, 1024);
    recv(sock, buffer, sizeof(buffer), 0);
    if (strlen(buffer) != 0){
      char* emailFrom = strtok(buffer, "|");
      char* emailHeader = strtok(NULL, "|");
      char* emailDesc = strtok(NULL, "|");
      printf(
        "\nReceiving from %s\nEmail Header: %s\nEmail Description:\n%s\n",
        emailFrom,
        emailHeader,
        emailDesc
        ); 

    }
    
}
/*we use fork to spawn a ne process then fork will return ID */
/*if id is 0 the it will be the child using the if statement the child will handle send while the parent will handle recieve*/
void handleServer(int sock){
  int id = fork();
  if (id == 0){
    while(1)
      handleSend(sock);
  }else{
    while(1)
      handleReceive(sock);
  }
}

/*show status if the client is connected to the server or not*/
void connectToServer(int sock, struct sockaddr_in addr){
  int value = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  if (value < 0){
      printf("Connection to the server failure. Exiting...");
      exit(1);
  }
  printf("Connected to the server.\n");
}

/*this function will establish that the clinet is the email and will connect to the server*/
int sendEmail(int sock){
    char message[1024];
    printf("Enter your email: ");
    fgets(message, 1024, stdin);
    ssize_t status = send(sock, message, strlen(message), 0);
    if (status == -1){
      printf("Something went wrong with establishing your email.");
      return -1;
    }
    return 0;
}