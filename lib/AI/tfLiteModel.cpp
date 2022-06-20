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
#include "tensorflow/lite/c/c_api_internal.h"

int hTrafficDicisionModel::initAIModel(const std::string modelPath, const std::string ifName){
    cout << "model uploading... " << modelPath << endl;
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
    // this->interpreter->SetInputs()
    if (status != kTfLiteOk){
      printf("allocate tensors fail\n");
      return -1;
    }else{
       printf("allocate tensors success code : %d\n", status);
    }
    
    // this->interpreter
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

int hTrafficDicisionModel::makeInputData(float *buffer){
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

    float bytes_max = max_v<uint32_t>(tx_bytes);
    float bytes_min = min_v<uint32_t>(tx_bytes);
    float bytes_mean =  mean_v<uint32_t>(tx_bytes);
    float packets_max = max_v<uint32_t>(tx_packets);
    float packets_min =  min_v<uint32_t>(tx_packets);
    float packets_mean = mean_v<uint32_t>(tx_packets);

    // cout << "tx_bytes : ";
    // printVector(tx_bytes);
    printf("%-15s : %15s\n","bytes_min", to_string((unsigned int)bytes_min).c_str());
    printf("%-15s : %15s\n","bytes_mean", to_string((unsigned int)bytes_mean).c_str());
    printf("%-15s : %15s\n","bytes_max", to_string((unsigned int)bytes_max).c_str());
    printf("%-15s : %15s\n","packets_min", to_string((unsigned int)packets_min).c_str());
    printf("%-15s : %15s\n","packets_mean", to_string((unsigned int)packets_mean).c_str());
    printf("%-15s : %15s\n","packets_max", to_string((unsigned int)packets_max).c_str());

    float arr[6] = {bytes_min, bytes_mean, bytes_max, packets_min, packets_mean, packets_max};
    memcpy(buffer, arr, sizeof(float)*6);

    this->prevData.clear();
    this->prevData.insert(map.begin(), map.end());
    return 0;
}

int hTrafficDicisionModel::setInputData(float *arr){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return -1;
    }
    // TfLiteTensor input = this->interpreter->get
    auto* input_1 = this->interpreter->typed_input_tensor<float>(0);
    
    if (!input_1){
      printf("fail to get input 1 tensor\n");
      return -1;
    }
    memset(input_1, 0, sizeof(float)*6);

    /*for scaling*/
    arr[0] /= 9765815;
    arr[1] /= 28546629;
    arr[2] /= 56912189;
    arr[3] /= 4417;
    arr[4] /= 12463;
    arr[5] /= 24776;
    cout << "input: " << printArray(arr, 6) << endl;
    memcpy(input_1, arr, sizeof(float)*6);

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

float hTrafficDicisionModel::getOutputData(void){
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

    cout << "Result is: " << *output << endl;
    return *output;
}

void hTrafficDicisionModel::printModelInOutInfo(void){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return;
    }

    cout << "[Model Total Info]" << endl;
    cout << "tensors size : " << this->interpreter->tensors_size() << endl;
    cout << "nodes size : " << this->interpreter->nodes_size() << endl;
    cout << "inputs size : " << this->interpreter->inputs().size() << endl;
    cout << "outputs size : " << this->interpreter->outputs().size() << endl;
    cout << endl << endl;

    vector<int32_t> vector = this->interpreter->inputs();
    cout << "[Model Input Info]" << endl;
    cout << "data : ";
    printVector(vector);
    for (int i=0;i < vector.size();i++){
      cout << i+1 << ". ";
      cout << "name : "<< this->interpreter->GetInputName(i) << " / ";
      auto* tensor = this->interpreter->input_tensor(i);
      cout << "dims data : " << printArray<float>((float*)tensor->dims->data, tensor->bytes/sizeof(float)) << " / ";
      cout << "dims size : " << tensor->dims->size  << " / ";
      cout << "array size : " << tensor->bytes/sizeof(float) << " / ";
      cout << "bytes : " << tensor->bytes << " / ";
      cout << "type : " << TfLiteTypeGetName(tensor->type)  << " / ";
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
      cout << "type : " << TfLiteTypeGetName(tensor->type) << endl;
      
    }
    cout << endl << endl;
}
