#include <stdio.h>

#define MAX_JSON_SIZE 30000

int ubus_cmd_call(char* path, char* item, char* buffer, char* arg);
int ubus_cmd_just_call(char* path, char* item);