#include "process.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stringEx.h"

int systemEx(char *cmd, char** result, int* nResult){
    FILE *fp;
    char buf[LINE_BUF];
    
    fp = popen(cmd, "r");
    if (fp == NULL){
        printf("popen() 실패");
        return -1;
    }
    while(fgets(buf, LINE_BUF-1, fp)){
        if ((*nResult)+1 > MAX_LINE) return 1;
        if (strlen(buf) < 1) break;
        char *line = (char*)malloc(sizeof(char)*strlen(buf)+100);
        int err = strncpy_ex(line, buf, LINE_BUF); 
        if (err == -1) return;
        result[*nResult] = line;
        (*nResult)++;
    }
    return 0;
}