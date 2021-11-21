#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

/*using struct so that it be easier to access 
different viaribles using the struct declared totalClients*/
struct clientData{
  int socket_id;
  char email[100];
} clients[100];
int totalClients = 0;

void* handleClient(void*);
int listenClient(struct clientData);
int main(){

  char *ip = "192.168.56.103";
  /*use 110 for POP standard port list*/
  int port = 110;

  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");

  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n", port);

  listen(server_sock, 5);
  printf("Listening...\n");
  /*waiting for a client to connect to the scoket created
  when there is a client that connect to the socket it will open a new thread
  the thread will handle the client reciever and sending */
    while(1){
        bzero(buffer, 1024);
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+]Client %d connected.\n", client_sock);
        printf("[+]Client establishing email...\n");
        recv(client_sock, buffer, sizeof(buffer), 0);
        int index = totalClients++;
        clients[index].socket_id = client_sock;
        strcpy(clients[index].email, buffer);
        printf("[+]Client email established as %s...\n", clients[index].email);
        pthread_t thread;
        pthread_create(&thread, NULL, handleClient, &clients[index]);
        printf("[+]Ready to connect to another client\n");
    }


  return 0;
}
/*wait for client to send a massage*/
int listenClient(struct clientData client){
  char buffer[1024];
  printf("Listening to %d\n", client.socket_id);
  ssize_t value = recv(client.socket_id, buffer, sizeof(buffer), 0);

/*value 0 means that the client disconnected from the server*/
  if (value == 0){
    printf("Client %d shutdown.\n", client.socket_id);
    return 0;
	/*value -1 means that error */
  }else if (value == -1){
    printf("Error on listening to client %d\n", client.socket_id);
    return 0;
	/*when it get actual data it will try to find the email, the client that i stored in the client
	it will search which one of them matches the on that the server is trying to find
	after it find the email it will return the email if not it will send a failure massage*/
  }else{
    if (strlen(buffer) > 0){
      char copied[1024];
      strcpy(copied, buffer);
      char* token = strtok(buffer, "|");
      printf("Receiving from %s, sending to %s\n", client.email, token);
      for(int i = 0; i < 100; i++){
        struct clientData currentClient = clients[i];
        if (strcmp(currentClient.email, token) == 0){
          send(currentClient.socket_id, copied, strlen(copied), 0);
          printf(
            "Successful sending to socket %d with email %s\n", 
            currentClient.socket_id, 
            currentClient.email
          );
          return 1;
        }
      }
      printf("Failure to send to client with email %s\n", token);
    }
    return 1;
  }
}
/*handleclient is the function that gets called in the thread, client listen will return 0 to end the listening
it will cause the hadleclient to close the connection*/
void* handleClient(void* value){
      struct clientData* clientd = (struct clientData*) value;
      struct clientData client = *clientd;
      printf("[+]Starting to listen client %d...\n", client.socket_id);
      while(1){
        // if listen returns 0, i want it to terminate
        if (listenClient(client) == 0)
          break;
      
      }
      printf("[+]Closing client %d...\n", client.socket_id);
      close(client.socket_id);

}