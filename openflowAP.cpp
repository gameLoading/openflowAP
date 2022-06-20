#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include "tools/myprocess.h"
#include "tools/myUnixSocketServer.h"
#include "tools/myqueue.h"
#include "wireless/wdev.h"
#include "AI/tfLiteModel.h"
#include "tools/mycppTools.h"
#include <iostream>
#include <queue>

#define FILENAME "openflowAP.c"

static Queue* q;

void* cmd_poll_func(void* model)
{
    while(1)
    {
        printf("polling queue...\n");
        int isDequeued = 0;
        while(!isEmpty(q)) 
        {
            char* data = dequeue(q);
            printf("꺼낸거 : %s\n", data);
            
            if (strncmp(data, "uci", 3)==0){
                isDequeued = 1;
                system(data);
            }
            free(data); 
        }
        if (isDequeued) {
            system("uci commit");
            system("ubus call network reload");
        }
        sleep(5);   
    }
}

void* wireless_poll_func(void *p){
    hTrafficDicisionModel *model = (hTrafficDicisionModel*)p;
    model->printModelInOutInfo();

    float buffer[6];
    deque<float> q;
    // model->setInputData();
    // model->invoke();
    // model->getOutputData();

    while(1)
    {
        printTime();
        model->makeInputData(buffer);
        model->setInputData(buffer);
        model->invoke();
        float value = model->getOutputData();
        q.push_front(value >= 0.1 ? 1 : 0);
        if (q.size() == 4) q.pop_back();
        printQueue(q);
        sleep(10);
    }
    return 0;
}

void* cmd_input_func(void* fd)
{
    int* sockfd = (int*)fd;
    char buf[MAX_BUF_SIZ]; 

    while(1)
    {
        memset(buf, 0x00, MAX_BUF_SIZ);
        if (read(*sockfd, buf, MAX_BUF_SIZ) <= 0) {
            break;
        }
        printf("RECV-> %s\n", buf);
        char *tmp =  (char*)malloc(sizeof(char)*strlen(buf));
        strcpy(tmp, buf);
        enqueue(q, tmp);
        // write(sockfd, buf, strlen(buf));r
    }
    close(*sockfd);
    *sockfd = -1;
    return 0;
}

int main(){
    hTrafficDicisionModel model;
    model.initAIModel(MODELNAME, "wlan0");
    printf("Tensorflow version : %s\n", TfLiteVersion());
    
    q = createQueue();
    pthread_t tid_cmd;
    if (pthread_create(&tid_cmd, NULL, cmd_poll_func, &model) == -1)
    {
        perror("pthread create error.\n");
    }

    pthread_t tid_wireless;
    if (pthread_create(&tid_wireless, NULL, wireless_poll_func, &model) == -1)
    {
        perror("pthread create error.\n");
    }

    startServer(cmd_input_func);
    freeQueue(q);
    return 0;
}

