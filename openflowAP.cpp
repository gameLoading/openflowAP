#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <iostream>
#include <queue>

#include "tools/myprocess.h"
#include "tools/myUnixSocketServer.h"
#include "tools/myqueue.h"
#include "wireless/wdev.h"
#include "AI/tfLiteModel.h"
#include "tools/mycppTools.h"
#include "wireless/wdev_extend.h"

#define FILENAME "openflowAP.c"
#define WIRELESS_POLLING_TIME_SEC 10

deque<string> cmd_queue(3);
static wireless_controller controller;
bool AI_CONTROLL = true;


void* cmd_poll_func(void* p)
{
    wireless_controller* controller = (wireless_controller*)p;
    while(1)
    {
        // printf("polling queue...\n");
        bool isDequeued = false;
        while(!cmd_queue.empty()) 
        {
            // char* data = dequeue(q);
            string uci = cmd_queue.back();
            cmd_queue.pop_back();
            printf("꺼낸거 : %s\n", uci.c_str());
            if (uci.find("uci")!= string::npos){
                isDequeued = true;
                if (uci.find("txpower") != string::npos){
                    int pos = uci.find("=");
                    string txpower_str = uci.substr(pos+1, (uci.length()-pos-1));
                    int txpower = stoi(txpower_str);
                    controller->set_txpower(txpower);
                }else{
                    system(uci.c_str());
                }
            }
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
    model_input_type buffer[6];
    memset(buffer, 0, sizeof(model_input_type)*6);
    deque<int> q;
    int host_count = 0;

    while(1)
    {
        int ret = model->makeInputData(buffer, &host_count);
        if (ret == -1){
            cout << "wating for delta data" << endl;
            sleep(WIRELESS_POLLING_TIME_SEC);
            continue;
        }
        printTime();
        model->printData(buffer, host_count);
        model->setInputData(buffer);
        model->invoke();
        model_output_type output = model->getOutputData();
        cout << "Result is: " << output << endl;
        q.push_front(output >= 0.01 ? 1 : 0);
        if (q.size() == 4) q.pop_back();
        printQueue(q);
        if (AI_CONTROLL){
            if (q.size() == 3 ){
                if (q.at(0) && q.at(1) && q.at(2)){
                    bool isSuccess = controller.up_txpower();
                    if (isSuccess){
                        cout << "AI detect user traffic" << endl;
                        cout << "AI increases radio tx power" << endl;
                    }
                }else if((q.at(0) | q.at(1) | q.at(2)) == 0){
                    bool isSuccess = controller.down_txpower();
                    if (isSuccess){
                        cout << "AI detect void time" << endl;
                        cout << "AI reduces radio tx power" << endl;
                    }
                }
            }
        }
        cout << "current tx power is " << controller.get_current_txpower()  << "dBm" << endl;
        sleep(WIRELESS_POLLING_TIME_SEC);
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
        cmd_queue.push_front(string(buf));
        // write(sockfd, buf, strlen(buf));r
    }
    close(*sockfd);
    *sockfd = -1;
    return 0;
}

int main(int argc, char* argv[]){
    for (int i=0;i < argc;i++){
        if (string(argv[i]).find("--nocontrol")!=string::npos){
            AI_CONTROLL = false;
        }
    }

    hTrafficDicisionModel model;
    model.initAIModel(MODELNAME, "wlan0");
    printf("Tensorflow version : %s\n", TfLiteVersion());

    controller.init_controller("wlan0");
    pthread_t tid_cmd;
    if (pthread_create(&tid_cmd, NULL, cmd_poll_func, &controller) == -1)
    {
        perror("pthread create error.\n");
    }

    pthread_t tid_wireless;
    if (pthread_create(&tid_wireless, NULL, wireless_poll_func, &model) == -1)
    {
        perror("pthread create error.\n");
    }

    startServer(cmd_input_func);
    return 0;
}

