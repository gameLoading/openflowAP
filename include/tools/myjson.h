#include <json-c/json.h>


typedef struct json_object JSONObject;

void addString(JSONObject* root, char *key, char *value);
void addString_array(JSONObject* arr, char *value);
void freeJSON(JSONObject* root);
void jsonFreeTest();