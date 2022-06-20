#include <string>
#include <memory>
#include <utility>
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <map>
#include <tuple>
#include <cstdint>
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/interpreter_builder.h"

// #define MODELNAME "/tmp/model/test_double_model.tflite"
#define MODELNAME "/tmp/model/openWRT_tflite_model.tflite"

using namespace std;
class hTrafficDicisionModel{
private:
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::string ifName;
    map<string, tuple<uint32_t, uint32_t>> prevData;
    void collectRawData(map<string, tuple<uint32_t, uint32_t>> *map);
public:
  hTrafficDicisionModel(){
    this->interpreter = nullptr;
    this->model = nullptr;
  }
  int initAIModel(const std::string modelPath, const std::string ifName);
  void printModelInOutInfo(void);
  int makeInputData(float *buffer);
  int setInputData(float *arr);
  float getOutputData(void);
  void invoke(void);
};