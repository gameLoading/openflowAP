#include "tools/myprocess.h"
#include <stdio.h>
#include <string.h>
#include "tools/mystrTools.h"
// #include "tools/myleak_detector_c.h"
#include <unistd.h>
#include <stdlib.h>

int systemEx(char *cmd, char** result, int* nResult){
    FILE *fp;
    
    fp = popen(cmd, "r");
    if (fp == NULL){
        printf("popen() 실패");
        return -1;
    }else{    
        char buf[LINE_BUF];
        while(fgets(buf, LINE_BUF-1, fp)){
            if ((*nResult)+1 > MAX_LINE) goto fail;
            if (strlen(buf) < 1) break;
            char *line = (char*)malloc(sizeof(char)*strlen(buf)+100);
	        memset(line, 0, sizeof(char)*strlen(buf)+100);
            int err = copyString_m(line, buf, strlen(buf)-1); 
            if (err == -1) goto fail;
            result[*nResult] = line;
            (*nResult)++;
        }
        goto success;
    }
success:
    // exit(0);
    if (fp) pclose(fp);
    fp = 0;
    return 0;
fail:
    if (fp) pclose(fp);
    fp = 0;
    return -1;
}

int systemEx_oneline(char *cmd, char* buf, int buf_len){
    FILE *fp = NULL;
    
    fp = popen(cmd, "r");
    if (fp == NULL){
        printf("popen() 실패");
        return -1;
    }else{
        int i=0;
        while(fgets(buf, buf_len-1, fp)){
            // printf("%s\n", buf);
            // return 0;
        }
        pclose(fp);
        fp = NULL;
    }
    return -1;
}

void printHeapMemory(void){
    char cmd[COMMAND_LEN]="";
    sprintf(cmd, "cat /proc/%d/status | grep VmData", getpid());
    system(cmd);
}