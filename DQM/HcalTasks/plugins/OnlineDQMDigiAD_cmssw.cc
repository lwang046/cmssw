/*
 * OnlineDQMDigiAD_cmssw.cpp
 *
 * Created on: Jun 10, 2023
 * Author: Mulugeta W.Asres, UiA, Norway
 *
 * The implementation follows https://github.com/cms-sw/cmssw/tree/master/PhysicsTools/ONNXRuntime
 */

// #include "FWCore/Utilities/interface/Exception.h"
// #include "FWCore/Utilities/interface/thread_safety_macros.h"
// #include "FWCore/Framework/interface/Event.h"
// #include "FWCore/Framework/interface/EDAnalyzer.h"
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "HeterogeneousCore/CUDAUtilities/interface/requireDevices.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include "DQM/HcalTasks/interface/OnlineDQMDigiAD_cmssw.h"

// using namespace std;
using namespace cms::Ort;

// Constructor
OnlineDQMDigiAD::OnlineDQMDigiAD(const std::string &modelFilepath, Backend backend) {
  std::string instanceName{"DESMOD Digioccupancy Map AD inference"};

  /**************** Initailize Model Memory States ******************/
  InitializeState();  // initailize model memory states to zero

  /**************** Create ORT session ******************/
  // Set up options for session

  auto session_options = ONNXRuntime::defaultSessionOptions(backend);
  // session_options = ONNXRuntime::defaultSessionOptions(backend);
  // Create session by loading the onnx model
  // std::string model_path = edm::FileInPath(modelFilepath).fullPath();
  model_path = edm::FileInPath(modelFilepath).fullPath();

  //ort_mSession = ONNXRuntime(model_path, &session_options);
  auto uOrtSession = std::make_unique<ONNXRuntime>(model_path, &session_options);
  ort_mSession = std::move(uOrtSession);

  std::cout << "******* model loading is success *******" << std::endl;
  // output_names = {"target_data", "pred_data", "pred_err_spatial_scaled", "pred_err_window_spatial_scaled", "pred_err_spatial_scaled_aml", "red_err_window_spatial_scaled_aml"};
}

void OnlineDQMDigiAD::IsModelExist(std::string subsystem_name) {
  assert(std::find(hcal_modeled_systems.begin(), hcal_modeled_systems.end(), subsystem_name) !=
         hcal_modeled_systems.end());
  std::cout << "onnx model integration is supported for the selected " << subsystem_name << " system!" << std::endl;
}

void OnlineDQMDigiAD::InitializeState() {
  // model memory states vectors init, only when the runs starts or for the first LS
  std::fill(input_model_state_memory_e_0_0.begin(),
            input_model_state_memory_e_0_0.end(),
            float(0.0));  // init model memory states-encoder_layer_0_state_0 to zero
  std::fill(input_model_state_memory_e_0_1.begin(),
            input_model_state_memory_e_0_1.end(),
            float(0.0));  // init model memory states-encoder_layer_0_state_1 to zero
  std::fill(input_model_state_memory_e_1_0.begin(),
            input_model_state_memory_e_1_0.end(),
            float(0.0));  // init model memory states-encoder_layer_1_state_0 to zero
  std::fill(input_model_state_memory_e_1_1.begin(),
            input_model_state_memory_e_1_1.end(),
            float(0.0));  // init model memory states-encoder_layer_1_state_1 to zero
  std::fill(input_model_state_memory_d_0_0.begin(),
            input_model_state_memory_d_0_0.end(),
            float(0.0));  // init model memory states-decoder_layer_0_state_0 to zero
  std::fill(input_model_state_memory_d_0_1.begin(),
            input_model_state_memory_d_0_1.end(),
            float(0.0));  // init model memory states-decoder_layer_0_state_1 to zero
  std::fill(input_model_state_memory_d_1_0.begin(),
            input_model_state_memory_d_1_0.end(),
            float(0.0));  // init model memory states-decoder_layer_1_state_0 to zero
  std::fill(input_model_state_memory_d_1_1.begin(),
            input_model_state_memory_d_1_1.end(),
            float(0.0));  // init model memory states-decoder_layer_1_state_1 to zero

  // model_state_refresh_counter = 15; // counter set due to onnx double datatype handling limitation that might cause precision error to propagate.
  model_state_refresh_counter =
      1;  // DQM multithread returns non-sequential LS. Hence, the model will not keep states (experimental)
}

std::vector<float> OnlineDQMDigiAD::Serialize2DVector(const std::vector<std::vector<float>> &input_2d_vec) {
  std::vector<float> output;
  for (const auto &row : input_2d_vec) {
    for (const auto &element : row) {
      output.push_back(element);
    }
  }
  return output;
}

std::vector<std::vector<float>> OnlineDQMDigiAD::Map1DTo2DVector(const std::vector<float> &input_1d_vec,
                                                                 const int numSplits) {
  std::size_t const splitted_size = input_1d_vec.size() / numSplits;
  // check splitted_size*numSplits == input_1d_vec.size()

  std::vector<std::vector<float>> output_2d_vec;

  for (size_t i = 0; i < input_1d_vec.size(); i += numSplits - 1) {
    std::vector<float> chunch_vec(input_1d_vec.begin() + i, input_1d_vec.begin() + i + splitted_size);
    output_2d_vec.push_back(chunch_vec);
  }
  return output_2d_vec;
}

std::vector<float> OnlineDQMDigiAD::PrepareONNXDQMMapVectors(
    std::vector<std::vector<std::vector<float>>> &digiHcal2DHist_depth_all) {
  std::vector<float> digi3DHistVector_serialized;

  for (std::vector<std::vector<float>> digiHcal2DHist_depth : digiHcal2DHist_depth_all) {
    std::vector<float> digiHcalDHist_serialized_depth = Serialize2DVector(digiHcal2DHist_depth);
    digi3DHistVector_serialized.insert(digi3DHistVector_serialized.end(),
                                       digiHcalDHist_serialized_depth.begin(),
                                       digiHcalDHist_serialized_depth.end());
  }

  return digi3DHistVector_serialized;
}

std::vector<std::vector<std::vector<float>>> OnlineDQMDigiAD::ONNXOutputToDQMHistMap(
    const std::vector<std::vector<float>> &ad_model_output_vectors, const int selOutputIdx) {
  // each output_vector is a serialized 3d hist map
  const unsigned short numDepth = 7;
  const unsigned short numDIeta = 64;

  // for (size_t i: selOutputIndices)
  // // for (std::vector<float> output_vector : ad_model_output_vectors)
  // {
  std::vector<float> output_vector = ad_model_output_vectors[selOutputIdx];
  std::vector<std::vector<float>> output_2d_vec = Map1DTo2DVector(output_vector, numDepth);

  std::vector<std::vector<std::vector<float>>> digiHcal3DHist;
  for (std::vector<float> output_vector_depth : output_2d_vec) {
    std::vector<std::vector<float>> digiHcal2DHist_depth = Map1DTo2DVector(output_vector_depth, numDIeta);
    digiHcal3DHist.push_back(digiHcal2DHist_depth);
  }
  // }

  return digiHcal3DHist;
}

// Perform inference for a given dqm map
std::vector<std::vector<float>> OnlineDQMDigiAD::Inference(std::vector<float> &digiHcalMapTW,
                                                           const std::vector<float> &numEvents,
                                                           const std::vector<float> &adThr,
                                                           std::vector<float> &input_model_state_memory_e_0_0,
                                                           std::vector<float> &input_model_state_memory_e_0_1,
                                                           std::vector<float> &input_model_state_memory_e_1_0,
                                                           std::vector<float> &input_model_state_memory_e_1_1,
                                                           std::vector<float> &input_model_state_memory_d_0_0,
                                                           std::vector<float> &input_model_state_memory_d_0_1,
                                                           std::vector<float> &input_model_state_memory_d_1_0,
                                                           std::vector<float> &input_model_state_memory_d_1_1) {
  /**************** Preprocessing ******************/
  // Create input tensor (including size and value) from the loaded inputs
  // Compute the product of all input dimension
  // Assign memory for input tensor
  // inputTensors will be used by the Session Run for inference

  const unsigned batch_size = 1;  // number sample to  be evaluated at once, a single time-window

  // typedef std::vector<std::vector<float>> FloatArrays;
  // FloatArrays input_values {digiHcalMapTW, numEvents, adThr};

  std::vector<std::vector<float>> input_values;
  input_values.emplace_back(digiHcalMapTW);
  input_values.emplace_back(numEvents);
  input_values.emplace_back(adThr);
  input_values.emplace_back(input_model_state_memory_e_0_0);
  input_values.emplace_back(input_model_state_memory_e_0_1);
  input_values.emplace_back(input_model_state_memory_e_1_0);
  input_values.emplace_back(input_model_state_memory_e_1_1);
  input_values.emplace_back(input_model_state_memory_d_0_0);
  input_values.emplace_back(input_model_state_memory_d_0_1);
  input_values.emplace_back(input_model_state_memory_d_1_0);
  input_values.emplace_back(input_model_state_memory_d_1_1);

  std::vector<std::vector<float>> outputs;

  /**************** Inference ******************/

  CPPUNIT_ASSERT_NO_THROW(outputs = ort_mSession->run(input_names, input_values, {}, output_names, batch_size));

  CPPUNIT_ASSERT(outputs.size() == output_names.size());
  CPPUNIT_ASSERT(outputs[0].size() == batch_size);

  return outputs;
}

// AD method to be called by the CMS system
std::vector<std::vector<float>> OnlineDQMDigiAD::Inference_CMSSW(
    std::string subsystem_name,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_1,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_2,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_3,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_4,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_5,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_6,
    const std::vector<std::vector<float>> &digiHcal2DHist_depth_7,
    const float LS_numEvents,
    const float flagDecisionThr)

{
  // check model availability
  hcal_subsystem_name = subsystem_name;
  IsModelExist(hcal_subsystem_name);  // assert model name

  /**************** Prepare data ******************/
  // merging all 2d hist into one 3d depth[ieta[iphi]]

  std::vector<std::vector<std::vector<float>>> digiHcal2DHist_depth_all;

  if (hcal_subsystem_name == "he") {
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_1);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_2);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_3);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_4);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_5);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_6);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_7);
  }

  else if (hcal_subsystem_name == "hb") {
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_1);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_2);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_3);
    digiHcal2DHist_depth_all.push_back(digiHcal2DHist_depth_4);
  }

  // convert the 3d depth[ieta[iphi]] vector into 1d and commbined
  std::vector<float> digiHcalMapTW = PrepareONNXDQMMapVectors(digiHcal2DHist_depth_all);

  const std::vector<float> adThr{flagDecisionThr};  // AD decision threshold, increase to reduce sensitivity
  const std::vector<float> numEvents{LS_numEvents};

  // call model inference
  /**************** Inference ******************/
  std::vector<std::vector<float>> output_tensors = Inference(digiHcalMapTW,
                                                             numEvents,
                                                             adThr,
                                                             input_model_state_memory_e_0_0,
                                                             input_model_state_memory_e_0_1,
                                                             input_model_state_memory_e_1_0,
                                                             input_model_state_memory_e_1_1,
                                                             input_model_state_memory_d_0_0,
                                                             input_model_state_memory_d_0_1,
                                                             input_model_state_memory_d_1_0,
                                                             input_model_state_memory_d_1_1);

  // auto output_tensors = Inference(digiHcalMapTW, numEvents, adThr);
  std::cout << "******* model inference is success *******" << std::endl;

  // // print the output data
  // std::vector<std::vector<float>> ad_model_output;
  // for (const auto &output_tensor : output_tensors)
  // {
  //     std::cout << "output array size: " << output_tensor.size() << std::endl;
  //     ad_model_output.emplace_back(output_tensor);
  // }

  /**************** Output post processing ******************/
  if (output_names.size() != output_tensors.size()) {
    std::cout << "Output vectors size must have the same size with output names." << std::endl;
    return std::vector<std::vector<float>>();
  }

  //  split outputs into ad output vectors and state_memory vectors
  std::string state_output_name_tag = "rnn_hidden";
  std::vector<std::vector<float>> ad_model_output_vectors, ad_model_state_vectors;
  for (size_t i = 0; i < output_tensors.size(); i++) {
    std::string output_names_startstr = output_names[i].substr(
        2, state_output_name_tag.length());  // Extract the same number of characters as str2 from mOutputNames
    if (output_names_startstr == state_output_name_tag) {
      std::cout << output_names[i] << ": state output array size: " << output_tensors[i].size() << std::endl;
      ad_model_state_vectors.emplace_back(output_tensors[i]);
    } else {
      std::cout << output_names[i] << ": ad output array size: " << output_tensors[i].size() << std::endl;
      ad_model_output_vectors.emplace_back(output_tensors[i]);
    }
  }

  const int num_state_vectors = 8;  // number of model state vectors
  if (ad_model_output_vectors.size() != num_state_vectors) {
    std::cout << "The number of output state vectors does not equals to expected." << std::endl;
    return std::vector<std::vector<float>>();
  }

  input_model_state_memory_e_0_0 = ad_model_state_vectors[0];
  input_model_state_memory_e_0_1 = ad_model_state_vectors[1];
  input_model_state_memory_e_1_0 = ad_model_state_vectors[2];
  input_model_state_memory_e_1_1 = ad_model_state_vectors[3];
  input_model_state_memory_d_0_0 = ad_model_state_vectors[4];
  input_model_state_memory_d_0_1 = ad_model_state_vectors[5];
  input_model_state_memory_d_1_0 = ad_model_state_vectors[6];
  input_model_state_memory_d_1_1 = ad_model_state_vectors[7];

  // # if onnx is returning serialized 1d vectors instead of vector of 3d vectors
  // aml score and flag are at index 5 and 7 of the vector ad_model_output_vectors: anomaly score: ad_model_output_vectors[5], anomaly flags: ad_model_output_vectors[7]
  /*
    selOutputIdx: index to select of the onnx output. e.g. 5 is the anomaly score and 7 is the anomaly flag (1 is with anomaly, 0 is healthy)
    std::vector<std::vector<std::vector<float>>> digiHcal3DHist_ANOMALY_FLAG = ONNXOutputToDQMHistMap(ad_model_output_vectors, 7)
    std::vector<std::vector<std::vector<float>>> digiHcal3DHist_ANOMALY_SCORE = ONNXOutputToDQMHistMap(ad_model_output_vectors, 5)
    */

  // reduce counter for each ls call. due to onnx double datatype handling limitation that might cause precision error to propagate.
  --model_state_refresh_counter;
  if (model_state_refresh_counter == 0)
    InitializeState();

  return ad_model_output_vectors;
}
