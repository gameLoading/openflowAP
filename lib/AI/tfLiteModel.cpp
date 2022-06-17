#include "AI/tfLiteModel.h"
#include <string>
#include <memory>
#include <utility>
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <json/json.h>
#include <string>
#include <tuple>
#include <map>
#include <cstdint>
#include "tools/mycppTools.h"
#include "wireless/wdev.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/interpreter_builder.h"

int hTrafficDicisionModel::initAIModel(const std::string modelPath, const std::string ifName){
    this->model = tflite::FlatBufferModel::BuildFromFile(modelPath.c_str());
    this->ifName = ifName;

    if(!this->model){
      printf("Fail to build model\n");
      return -1;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    TfLiteStatus status = tflite::InterpreterBuilder(*(this->model).get(), resolver)(&this->interpreter);
    if (status != kTfLiteOk){
      printf("interpreter build fail\n");
      return -1;
    }else{
      printf("interpreter build success code : %d\n", status);
    }

    status = this->interpreter->AllocateTensors();
    if (status != kTfLiteOk){
      printf("allocate tensors fail\n");
      return -1;
    }else{
       printf("allocate tensors success code : %d\n", status);
    }
    
    return 0;
}

void hTrafficDicisionModel::collectRawData(
  map<string, tuple<uint32_t, uint32_t>> *map){
    int host_count;
    char buffer[MAX_JSON_SIZE];
  
    Json::Value root, result, host;
    getAssocicatedHosts((char*)this->ifName.c_str(), buffer, MAX_JSON_SIZE);

    string json_str = buffer;

    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool is_parsed = reader->parse(json_str.c_str(), json_str.c_str()+json_str.length(),&root, &err);
    if (!is_parsed){
      cout << "failed to parse JSON" << endl;
    }
    
    try {
      if (root.isMember("results")){
          result = root.get("results", "");
          host_count = result.size();
          if (host_count == 0) return;
          for (int i=0;i < host_count;i++){
            host = result[i];
            // cout << host.toStyledString() << endl;
            (*map)[host["mac"].asString()] = tuple<uint32_t, uint32_t>(host["tx"]["bytes"].asUInt(), host["tx"]["packets"].asUInt());
            // cout << "key : " << host["mac"].asString() << " / value : "<< get<0>((*map)[host["mac"].asString()]) << endl;
          }
      }
    }catch(Json::LogicError error){
      cout << "error msg : " << error.what() << endl;
    }
}

int hTrafficDicisionModel::makeInputData(tuple<float, float, float, float, float, float> *buffer){
    // buffer->clear();
    map<string, tuple<uint32_t, uint32_t>> map;
    collectRawData(&map);

    if (this->prevData.empty()){
        this->prevData.clear();
        this->prevData.insert(map.begin(), map.end());
        return -1;
    }

    vector<uint32_t> tx_packets;
    vector<uint32_t> tx_bytes;

    for (auto iter_cur : map){
      for (auto iter_prev : this->prevData){
        if (iter_cur.first == iter_prev.first){
          tx_bytes.push_back(get<0>(iter_cur.second) - get<0>(iter_prev.second));
          tx_packets.push_back(get<1>(iter_cur.second) - get<1>(iter_prev.second));
        }
      }
      // cout << iter.first.c_str() << " / " << get<0>(iter.second) << endl;
    }

    cout << "tx_bytes : ";
    printVector(tx_bytes);
    cout << "bytes_max : " << max_v<uint32_t>(tx_bytes) << endl;
    cout << "bytes_min : " << min_v<uint32_t>(tx_bytes) << endl;
    cout << "bytes_mean : " << mean_v<uint32_t>(tx_bytes) << endl;
    cout << "tx_packets : ";
    printVector(tx_packets);
    cout << "packets_max : " << max_v<uint32_t>(tx_packets) << endl;
    cout << "packets_min : " << min_v<uint32_t>(tx_packets) << endl;
    cout << "packets_mean : " << mean_v<uint32_t>(tx_packets) << endl;

    this->prevData.clear();
    this->prevData.insert(map.begin(), map.end());
    return 0;
}

int hTrafficDicisionModel::setInputData(tuple<float, float, float, float, float, float> data){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return -1;
    }
    auto* input = this->interpreter->typed_input_tensor<int64_t>(0);

    if (!input){
      printf("fail to get input tensor\n");
      return -1;
    }
    // Dummy input for testing
    *input = 2;
    return 0;
}

void hTrafficDicisionModel::invoke(void){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return;
    }
    this->interpreter->Invoke();
}

int hTrafficDicisionModel::getOutputData(void){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return -1;
    }

    auto* output = this->interpreter->typed_output_tensor<float>(0);
    if (!output) {
      printf("fail to get output tensor\n");
      return -1;
    }

    printf("Result is: %f\n", *output);
    return 0;
}

void hTrafficDicisionModel::printModelInOutInfo(void){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return;
    }
    vector<int32_t> vector = this->interpreter->inputs();
    cout << "[Model Input Info]" << endl;
    cout << "data : ";
    printVector(vector);
    for (int i=0;i < vector.size();i++){
      cout << i+1 << ". ";
      cout <<"name : "<< this->interpreter->GetInputName(i) << " / ";
      auto* tensor = this->interpreter->input_tensor(i);
      cout << "type : " << tensor->type << endl;
    }
    cout << endl << endl;

    vector = this->interpreter->outputs();
    cout << "[Model Ouput Info]" << endl;
    cout << "data : ";
    printVector(vector);
    for (int i=0;i < vector.size();i++){
      cout << i+1 << ". ";
      cout << "name : " << this->interpreter->GetOutputName(i) << " / ";
      auto* tensor = this->interpreter->output_tensor(i);
      cout << "type : " << tensor->type << endl;
    }
    cout << endl << endl;
}
