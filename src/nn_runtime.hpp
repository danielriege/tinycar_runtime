#pragma once

#include <string>
#include <opencv2/core.hpp>

class NNRuntime {
public:
    virtual ~NNRuntime() {};

    virtual int loadModel(const std::string& path) = 0;
    virtual int run(float*  outputBuffer, cv::Mat image) = 0;
    virtual cv::Size getInputSize() = 0;
};