#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "tools/myqueue.h"
#include "tools/myUnixSocketServer.h"
// #include "tools/myleak_detector_c.h"
// #include "tools/mymemory.h"

#define TRUE 1
#define FALSE 0

#define FILENAME "openflowAP.c"
#define MAX_JSON_SIZE 30000


static Queue* q;
static int isInterrupted = FALSE;

void startServer(void* (*fp)(void*));

void startServer(void* (*fp)(void*)){
    int server_sockfd;
    int state, client_len;
    pid_t pid;
    
    int clientFds[MAX_CLIENT];
    struct sockaddr_un clientaddr, serveraddr;

    char buf[MAX_BUF_SIZ]; 

    if (access(UNIX_SOCK_FILE, F_OK) == 0) {
        unlink(UNIX_SOCK_FILE);
    }

     client_len = sizeof(clientaddr);
     if ((server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
         perror("socket error : ");
         exit(0);
     }
     bzero(&serveraddr, sizeof(serveraddr));
     serveraddr.sun_family = AF_UNIX;
     strcpy(serveraddr.sun_path, UNIX_SOCK_FILE);


     state = bind(server_sockfd , (struct sockaddr *)&serveraddr,
             sizeof(serveraddr));
     if (state == -1) {
         perror("bind error : ");
         exit(0);
     }
     state = listen(server_sockfd, MAX_CLIENT);
     if (state == -1) {
         perror("listen error : ");
         exit(0);
     }

    memset(clientFds, -1, sizeof(clientFds));
    while(1)
    {
        int* client_sockfd;
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (clientFds[i] == -1) client_sockfd = &clientFds[i];
        }
        printf("waiting for host\n");
        *client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr,
                (socklen_t*)&client_len);
        if (*client_sockfd == -1) {
            perror("Accept error : ");
            exit(0);
        }
        pthread_t en_tid;
        if (pthread_create(&en_tid, NULL, fp, client_sockfd) == -1)
        {
            perror("pthread create error.\n");
        }
    }
}