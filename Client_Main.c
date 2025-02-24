
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "UI/cssfrm.h" 
#include <sys/fcntl.h>

#define SERVER_ADDRESS "127.0.0.2"
#define SERVER_PORT 8080

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);

  Message("Client started, trying connectiion...");

  struct sockaddr_in serverADDR;
  int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket < 0) {
      Error("Failed to initialise server's socket");
      goto EXIT_WO_CLOSE;
    }
  serverADDR.sin_family = AF_INET;
  serverADDR.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_ADDRESS, &serverADDR.sin_addr) < 0) {
    Error("Failed to establish server IP");
    goto EXIT;
  }
  else {
    Message("\nServer found on:\n");
    printf("    ip: %s\n", inet_ntoa(serverADDR.sin_addr));
    printf("\n    port: %hu\n", ntohs(serverADDR.sin_port));
  }

  if(connect(serverSocket, (struct sockaddr*)&serverADDR, sizeof(serverADDR )) < 0) {
    Error("Connection failed");
    goto EXIT;
  } 
  Message("\nConnection established!\n");
/*========================================================================================*/
  fcntl(serverSocket, F_SETFL, O_NONBLOCK);
  int server_blockFL = fcntl(serverSocket, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    char response[] = "ok";
    char command[3];
    char dialog[2048] = {0};
    char input[4];

    while(1) {
      if(read(STDIN_FILENO, input, 3) > 0) {
        // if(!strncmp(input, "/M ", 3)) {
        //     //printf("\n/M detected ");
        //     scanf("%s", dialog);
        //     //printf("\n%s string detected", dialog);

        //         // command example:  /M <len><Dialog>
        //     send(serverSocket, input, 3, 0);
        //     int len = strlen(dialog) + 1;
        //     dialog[len+1] = 0;
        //     send(serverSocket, &len, 4, 0);
        //     send(serverSocket, dialog, strlen(dialog), 0);

        // }
        if(!strncmp(input, "/C\n", 3)) {
          goto EXIT;
        }
      }

      if(read(serverSocket, command, 3) > 0)
        if(strncmp(command, "^P", 3) == 0) send(serverSocket, response, 3, 0);
        // else if(strncmp(command, "/M ", 3) == 0) {
        //   fcntl(STDIN_FILENO, F_SETFL, server_blockFL);
        //   int len = 0;
        //   if(read(serverSocket, &len, 4) < 0) Error("Couldnt read msg LEN");
        //   if(read(serverSocket, dialog, len) < 0) Error("Couldnt read msg");


        //   else {
        //     printf("read message: %s\n", dialog);
        //   }
        //   fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        // }
    }


  EXIT:
  close(serverSocket);
  EXIT_WO_CLOSE:

  printf("\nClient is shut down!\n\n"); 
  return 0;
}