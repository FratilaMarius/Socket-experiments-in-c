#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <time.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <fcntl.h>

#include "UI/cssfrm.h" 

#define PORT 8080
#define PORT_HEX htons(PORT) // PORT in hex reversed, accepted by port structs
#define SERVER_IP "127.0.0.2"
#define SERVER_IP_HEX inet_addr(SERVER_IP)

#define PING_MSG "^P"
#define USR_MSG "/M"
#define NR_CLIENTS 15

#define SERV_SOCK_ERR 1
#define BIND_ERR 2
#define LISTEN_ERR 4
#define ACCEPT_ERR 8  
#define CLIENT_SOCK_ERR 16

typedef struct {
  int ID_user;
  struct sockaddr_in address_user;
}user;

long ping(int fd_client, char *msg, int len) {
  struct timespec wait_for, end, begin; wait_for.tv_nsec = 1000000; wait_for.tv_sec = 0;
  int attempts = 0;
  char rsp[3];

  clock_gettime(CLOCK_MONOTONIC, &begin); // chikenshit error to ignore because vscode cant be assed to check time.h macros properly. it works anyway
  if(send(fd_client, msg, len, 0) < 0) return -1;
  while(1) {
    if(attempts == 150) return -1;
    //printf("ping %d ", attempts);
    if(read(fd_client, rsp, 3) > 0) break;
    nanosleep(&wait_for, NULL);
    attempts++;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  return ((end.tv_sec - begin.tv_sec) * 1000 + (end.tv_nsec - begin.tv_nsec) / 1000000);
}

int main() {
  printf("Server starting...\n");
  
  setvbuf(stdout, NULL, _IONBF, 0);

  int addrlen = sizeof(struct sockaddr);

  short int error_check = 0; // lucrez pe bitii lui error check ca sa economisesc memorie
  int server_SockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
  if(server_SockId < 0) {
    error_check |= SERV_SOCK_ERR;
    goto SOCK_ERR;  
  } 
  else Message("\nServer socket created\n");
  
  struct sockaddr_in serverADDR;
    serverADDR.sin_family = AF_INET;
    serverADDR.sin_addr.s_addr = SERVER_IP_HEX; 
    serverADDR.sin_port = PORT_HEX;
      
    int opt = 1; // val needed for chikenshit syscall that needs a pointer to an integer
  if(setsockopt(server_SockId, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE, &opt, sizeof(opt))) {
    Error("'setsockopt()' failed!");
    goto EXIT;
  }
  
  if(bind(server_SockId, (struct sockaddr*)&serverADDR, sizeof(serverADDR)) < 0) {
    Error("Server socket bind failed!");
    error_check |= BIND_ERR;
    goto EXIT;
  }
  else {
    Message("\nServer is succsesfully bound!\n");
    printf("    server ip: %s\n", inet_ntoa(serverADDR.sin_addr));
    printf("\n    server port: %hu\n", ntohs(serverADDR.sin_port));
  }

  if(listen(server_SockId, 3) < 0) {
    Error("Listen failed");
    error_check |= LISTEN_ERR;
    goto EXIT;
  }
  printf("Listening on 127.0.0.1:%hu\n", ntohs(serverADDR.sin_port));
/////////////////////////////////////
//
//
/////////////////////////////////////
  user client[NR_CLIENTS];
  for(int i = 0; i < NR_CLIENTS; i++) {
    client[i].ID_user = -1;
  }
  char input = 0;
  char client_commands[3];
  char messages[2048] = {0};
  int currentFreeUserInList = 0;

  struct timespec tickRate;
    tickRate.tv_nsec = 1000000; // 1.5 sec
    tickRate.tv_sec = 0;

  if (fcntl(server_SockId, F_SETFL, O_NONBLOCK) < 0) {
    Error("'fcntl(O_NONBLOCK 1)' failed!");
    goto EXIT;
  }
  if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) < 0) {
    Error("'fcntl(O_NONBLOCK 2)' failed!");
    goto EXIT;
  }

  int rec = 0;
  while (1)
  {
    rec++;
    currentFreeUserInList = NR_CLIENTS + 1;
    if(read(STDIN_FILENO, &input, 1) > 0) goto EXIT_CLOSEALL; // see if we close server
    
    // printf("\nid user: %d %d %d", client[0].ID_user, client[1].ID_user, client[2].ID_user);
    for(int i = 0; i < NR_CLIENTS; i++) { // get the current free user in list towards 0
      if(client[i].ID_user < 0) {
        currentFreeUserInList = i;
        break;
      }
    }
    // printf("\nCURRENT FREE: %d", currentFreeUserInList);
    if(currentFreeUserInList != NR_CLIENTS + 1) { // assign a user a new id if a free one exists and connection is pending
      client[currentFreeUserInList].ID_user = accept(server_SockId, (struct sockaddr*)&client[currentFreeUserInList].address_user, &addrlen);
        if(client[currentFreeUserInList].ID_user >= 0) {
          fcntl(client[currentFreeUserInList].ID_user, F_SETFL, O_NONBLOCK);
          printf("\nClient connected on list id %d, ip %s and port %u.\n", currentFreeUserInList, inet_ntoa(client[currentFreeUserInList].address_user.sin_addr), htons(client[currentFreeUserInList].address_user.sin_port));
        }
        // else {
        //   Error("Accept failed");
        // }
    }
    else {
      int temp_sock = accept(server_SockId, (struct sockaddr*)&client[currentFreeUserInList].address_user, &addrlen);
      static char *msg = "\nSorry, we're full. Try again later\n";
      send(temp_sock, "msg", sizeof(msg)/sizeof(char), 0);
      close(temp_sock);
      printf("\nNo client space!\n");
    }

    for(int i = 0; i < NR_CLIENTS; i++) {
      if(client[i].ID_user > 0) {
        // messages:
        // if(read(client[i].ID_user, client_commands, 3) > 0) {
        //   if(!strncmp(client_commands, "/M ", 3)) {
        //     int len;
        //     read(client[i].ID_user, &len, 4);
        //     read(client[i].ID_user, messages, len);
        //     printf("\nClient said: %s\n", messages);

        //     for(int j = 0; j < NR_CLIENTS; j++) {
        //       if(i == j) continue;
              
        //       send(client[j].ID_user, "/M ", 3, 0); 
        //       send(client[j].ID_user, &len, 4, 0);
        //       send(client[j].ID_user, messages, len, 0);
        //     }
        //   }
        // }
        

        // ping:
        long response = ping(client[i].ID_user, PING_MSG, strlen(PING_MSG)+1);
        if(response < 0) {
          close(client[i].ID_user);
          client[i].ID_user = -1;
            printf("\nClosed a client on list id %d\n", i);
        }
        else 
          if(rec == 1000) 
            printf("Ping id_%d: %ldms\n", i, response);
      }
    }// ping and delete

    // read from all and see if there are new messages
    if(rec == 1000) 
      rec = 0;
    nanosleep(&tickRate, NULL);
  } 
  
EXIT_CLOSEALL:
for(int i = 0; i < NR_CLIENTS; i++)
  close(client[i].ID_user);
EXIT:
  close(server_SockId);
SOCK_ERR:

  printf("Server is shut down!\n\n");
  return 0;
}