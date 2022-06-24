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
#include "tools/mystrTools.h"
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
  map<string, tuple<int64_t, int64_t>> *map, int* host_count){
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
          *host_count = result.size();
          if (*host_count == 0) return;
          for (int i=0;i < *host_count;i++){
            host = result[i];
            // cout << host.toStyledString() << endl;
            (*map)[host["mac"].asString()] = tuple<int64_t, int64_t>(host["tx"]["bytes"].asLargestInt(), host["tx"]["packets"].asLargestInt());
            // cout << "key : " << host["mac"].asString() << " / value : "<< get<0>((*map)[host["mac"].asString()]) << endl;
          }
      }
    }catch(Json::LogicError error){
      cout << "error msg : " << error.what() << endl;
    }
}

int hTrafficDicisionModel::makeInputData(model_input_type *buffer, int *host_count){
    // buffer->clear();
    map<string, tuple<int64_t, int64_t>> map;
    this->collectRawData(&map, host_count);

    if (this->prevData.empty()){
        this->prevData.insert(map.begin(), map.end());
        return -1;
    }

    vector<int64_t> tx_packets;
    vector<int64_t> tx_bytes;

    for (auto iter_cur : map){
      for (auto iter_prev : this->prevData){
        if (iter_cur.first == iter_prev.first){
          tx_bytes.push_back(get<0>(iter_cur.second) - get<0>(iter_prev.second));
          tx_packets.push_back(get<1>(iter_cur.second) - get<1>(iter_prev.second));
        }
      }
      // cout << iter.first.c_str() << " / " << get<0>(iter.second) << endl;
    }
    
    this->prevData.clear();
    this->prevData.insert(map.begin(), map.end());

    model_input_type bytes_max = max_v<int64_t>(tx_bytes);
    model_input_type bytes_min = min_v<int64_t>(tx_bytes);
    model_input_type bytes_mean = mean_v<int64_t>(tx_bytes);
    model_input_type packets_max = max_v<int64_t>(tx_packets);
    model_input_type packets_min =  min_v<int64_t>(tx_packets);
    model_input_type packets_mean = mean_v<int64_t>(tx_packets);

    model_input_type arr[6] = {bytes_min, bytes_mean, bytes_max, packets_min, packets_mean, packets_max};
    memcpy(buffer, arr, sizeof(model_input_type)*6);
    return 0;
}

void hTrafficDicisionModel::printData(model_input_type *arr, int host_count){
    if((arr+5)==0) return;
    model_input_type bytes_min = arr[0];
    model_input_type bytes_mean = arr[1];
    model_input_type bytes_max = arr[2];
    model_input_type packets_min = arr[3];
    model_input_type packets_mean = arr[4];
    model_input_type packets_max = arr[5];
    
    cout << "host count : " << host_count << endl;
    printf("%-15s : %15s\n","bytes_min", number2comma((long)bytes_min));
    printf("%-15s : %15s\n","bytes_mean",  number2comma((long)bytes_mean));
    printf("%-15s : %15s\n","bytes_max",  number2comma((long)bytes_max));
    printf("%-15s : %15s\n","packets_min",  number2comma((long)packets_min));
    printf("%-15s : %15s\n","packets_mean",  number2comma((long)packets_mean));
    printf("%-15s : %15s\n","packets_max",  number2comma((long)packets_max));
    cout << "input: " << printArray(arr, 6) << endl;
}

int hTrafficDicisionModel::setInputData(model_input_type *arr){
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

model_output_type hTrafficDicisionModel::getOutputData(void){
    if (this->interpreter == nullptr) {
      printLine();
      cout << "interpreter is null" << endl;
      return -1;
    }

    auto* output = this->interpreter->typed_output_tensor<model_output_type>(0);
    if (!output) {
      printf("fail to get output tensor\n");
      return -1;
    }

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
      cout << "dims data : " << printArray<model_input_type>((model_input_type*)tensor->dims->data, tensor->bytes/sizeof(model_input_type)) << " / ";
      cout << "dims size : " << tensor->dims->size  << " / ";
      cout << "array size : " << tensor->bytes/sizeof(model_input_type) << " / ";
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

bool hTrafficDicisionModel::can_make_delta_data(void){
  return !this->prevData.empty();
}
