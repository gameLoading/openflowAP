// #include <json-c/json.h>
#include "tools/myjson.h"
#include <stdio.h>

void addString_object(JSONObject* root, char *key, char *value){
    JSONObject* object = json_object_new_object();
    json_object_set_string(object, value);
    json_object_object_add(root, key, object);
}

void addString_array(JSONObject* arr, char *value){
    json_object_array_add(arr, json_object_new_string(value));
}

void freeJSON(JSONObject* root){
    while(json_object_put(root) != 1) {}
}

void jsonFreeTest(){
    // char *p = (char*)malloc(sizeof(char)*100);
    // free(p);
    JSONObject* root = json_object_new_object();
    JSONObject* info = json_object_new_object();
    json_object_object_add(info, "key1", json_object_new_string("value1"));
    json_object_object_add(info, "key2", json_object_new_string("value3"));
    json_object_object_add(info, "key3", json_object_new_string("value3"));

    JSONObject* arr = json_object_new_array();
    json_object_array_add(arr, json_object_new_string("str1"));
    json_object_array_add(arr, json_object_new_string("str2"));
    json_object_array_add(arr, json_object_new_string("str3"));

    json_object_object_add(root, "info", info);
    json_object_object_add(root, "arr", arr);

    // freeJSON(info);
    // freeJSON(arr);
    // freeJSON(root);

    // printf("json_object_put(info); : %d\n", json_object_put(info));
    // printf("json_object_put(arr); : %d\n", json_object_put(arr));
    // printf("json_object_put(root); : %d\n", json_object_put(root));
    // printf("json_object_put(info); : %d\n", json_object_put(info));
    // printf("json_object_put(arr); : %d\n", json_object_put(arr));
    // printf("json_object_put(root); : %d\n", json_object_put(root));
    // root = NULL;
    // arr = NULL;
    info = NULL;
}