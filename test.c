#include <stdio.h>
#include <libubox/blobmsg_json.h>
#include <json-c/json.h>
#include "tools/process.h"
#include "test.h"

void test(){
    char dev[] = "wlan1";
    char test[] = "{\"device\":\"wlan1\"}"; 
    printf("try to %s\n", test);
    struct json_object *root = json_tokener_parse(test);
    printf("succes222s\n");
    // printf("%d", root);
    struct json_object* dev_json = json_object_object_get(root, "device");
    printf("succes212s\n");
    char* deviceName = json_object_get_string(dev_json);
    printf("succes232s\n");
    printf("%s", deviceName);
    printf("success\n");
}