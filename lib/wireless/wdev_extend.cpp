#include "wireless/wdev_extend.h"
#include <string>
#include <json/json.h>
#include <cstdlib>
#include <iostream>
#include "tools/mycppTools.h"
using namespace std;

void wireless_controller::init_controller(string device_name){
    this->device_name = device_name;
    this->txpower_list.clear();
    this->update_txpower_list();
    this->set_max_txpower();
}

bool wireless_controller::is_available_txpower(int txpower){
    // printVector(this->txpower_list);
    for (int element : this->txpower_list){
        if (element == txpower) return true;
    }
    return false;
}

int  wireless_controller::get_current_txpower(){
    string device_name = this->device_name;
    char result[MAX_JSON_SIZE];
    memset(result, 0, sizeof(char)*MAX_JSON_SIZE);
    getWirelessDeviceInfo((char*)device_name.c_str(), result, MAX_JSON_SIZE);

    Json::Value root, txpower;
    string json_str(result);
    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool is_parsed = reader->parse(json_str.c_str(), json_str.c_str()+json_str.length(),&root, &err);
    if (!is_parsed){
      cout << "failed to parse JSON" << endl;
      return -1;
    }
    
    try {
      if (root.isMember("txpower")){
          txpower = root.get("txpower", -1);
          return txpower.asInt();
      }
    }catch(Json::LogicError error){
      cout << "error msg : " << error.what() << endl;
      return -1;
    }
    return -1;
}

void wireless_controller::update_txpower_list(){
    char result[MAX_JSON_SIZE];
    memset(result, 0, sizeof(char)*MAX_JSON_SIZE);
    getTxpowerList((char *)device_name.c_str(), result, MAX_JSON_SIZE);

    Json::Value root, results, txpower_info;
    string json_str(result);
    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool is_parsed = reader->parse(json_str.c_str(), json_str.c_str()+json_str.length(),&root, &err);

    if (!is_parsed){
      cout << "failed to parse JSON" << endl;
      return;
    }
    
    try {
      if (root.isMember("results")){
          results = root.get("results", -1);
          if (results.size() == 0) return;

          for (int i=0;i < results.size();i++){
            int txpower = results[i].get("dbm", 0).asInt();
            this->txpower_list.push_back(txpower);
          }
      }
    }catch(Json::LogicError error){
      cout << "error msg : " << error.what() << endl;
      return;
    }
    return;
}


bool wireless_controller::up_txpower(){
    string device_name = this->device_name;
    int current_txpower = this->get_current_txpower();
    if (current_txpower == -1){
        // cout << "current txpower : " << current_txpower << " is wrong" << endl;
        return false;
    }
    if (!this->is_available_txpower(current_txpower+1)){
        // cout << "current txpower is maximum" << endl;
        return false;
    }
    // cout << "current txpower is " << current_txpower+1 << endl;
    this->set_txpower(current_txpower+1);
    return true;
}

bool wireless_controller::down_txpower(){
    string device_name = this->device_name;
    int current_txpower = get_current_txpower();
    if (current_txpower == -1){
        // cout << "current txpower : " << current_txpower << " is wrong" << endl;
        return false;
    }
    if ((current_txpower-1) < 0){
        // cout << "current txpower is minimum" << endl;
        return false;
    }
    // cout << "current txpower is " << current_txpower-1 << endl;
    this->set_txpower(current_txpower-1);
    return true;
}

bool wireless_controller::is_broadcom_chipset(){
    char result[MAX_JSON_SIZE];
    memset(result, 0, sizeof(char)*MAX_JSON_SIZE);
    getSystemBoard((char*)device_name.c_str(), result, MAX_JSON_SIZE);

    Json::Value root;
    string json_str(result);
    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool is_parsed = reader->parse(json_str.c_str(), json_str.c_str()+json_str.length(),&root, &err);
    if (!is_parsed){
      cout << "failed to parse JSON" << endl;
      return -1;
    }
    
    try {
      if (root.isMember("release")){
          string target = root["release"]["target"].asString();
          if (target.find("bcm") != string::npos) return true;
      }
    }catch(Json::LogicError error){
      cout << "error msg : " << error.what() << endl;
      return false;
    }
    return false;
}

void wireless_controller::set_max_txpower(){
    int max_txpower = 0;
    for (int txpower : this->txpower_list){
      if (txpower > max_txpower) max_txpower = txpower;
    }
    this->set_txpower(max_txpower);
} 
void wireless_controller::set_txpower(int txpower){
    char command[MAX_UCI_LEN];
    if (this->is_broadcom_chipset()){
        txpower *= 100;
    }
    sprintf(command, "iw dev %s set txpower fixed %d", this->device_name.c_str(), txpower);                                                       
    system(command);
}