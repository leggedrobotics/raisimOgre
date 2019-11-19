//
// Created by jemin on 11/9/19.
// MIT License
//
// Copyright (c) 2019-2019 Robotic Systems Lab, ETH Zurich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef RAISIMOGRE_SLIDINGMEMORY_HPP
#define RAISIMOGRE_SLIDINGMEMORY_HPP

#include <memory>
#include "raisim/World.hpp"

namespace raisim {

class SlidingMemory {
 public:

  SlidingMemory (int window_size, float initial_value) {
    RSFATAL_IF(window_size>400, "Currently, RaisimOgre cannot handle more than 400 data points due to the index data type.")
    windowSize_ = window_size;
    writePosition_ = window_size;
    memory_.resize(windowSize_*reserveFactor_, initial_value);
    defaultValue_ = initial_value;
  }
  
  void push_back(float value) {
    if(writePosition_==windowSize_*reserveFactor_) {
      memcpy(&memory_[0], &memory_[windowSize_*(reserveFactor_-1)], sizeof(float)*windowSize_);
      writePosition_ = windowSize_;
      displayPosition_ = 0;
      memory_[writePosition_] = value;
    } else {
      memory_[writePosition_] = value;
    }

    displayPosition_++;
    writePosition_++;
  }
  
  size_t size() {
    return windowSize_;
  }

  float* data() {
    return &memory_[displayPosition_];
  }

  void clear() {
    for(auto& ele: memory_)
      ele = defaultValue_;
  }
  
 private:
  size_t writePosition_ = 0, displayPosition_ = 0, windowSize_;
  std::vector<float> memory_;
  float defaultValue_;
  static int constexpr reserveFactor_ = 4;
};

}

#endif //RAISIMOGRE_SLIDINGMEMORY_HPP
