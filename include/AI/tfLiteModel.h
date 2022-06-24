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
#define MODELNAME "/root/model/openWRT_tflite_model.tflite"

typedef float model_output_type;
typedef float model_input_type;

using namespace std;
class hTrafficDicisionModel{
private:
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::string ifName;
    map<string, tuple<int64_t, int64_t>> prevData;
    void collectRawData(map<string, tuple<int64_t, int64_t>> *map, int* host_count);
public:
  hTrafficDicisionModel(){
    this->interpreter = nullptr;
    this->model = nullptr;
  }
  int initAIModel(const std::string modelPath, const std::string ifName);
  void printModelInOutInfo(void);
  void printData(model_input_type *arr, int host_count);
  int makeInputData(model_input_type *buffer, int *host_count);
  int setInputData(model_input_type *arr);
  model_output_type getOutputData(void);
  void invoke(void);
  bool can_make_delta_data(void);
};