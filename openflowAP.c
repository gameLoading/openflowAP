#include <stdio.h>
#include <libubox/blobmsg_json.h>
#include <json-c/json.h>
#include "tools/process.h"
#include "test.h"

int main(){
    // test();
    int nLine = 0;
    char **result = (char**)malloc(sizeof(char*)*MAX_LINE); 
    systemEx("uci show wireless | grep radio0", result, &nLine);

    for (int i=0;i<nLine;i++){
        printf("%s\n", result[i]);
    }
    free(result);
    return 0;
}