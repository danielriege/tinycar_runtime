#pragma once

#include <memory>
#include "../../nn_runtime.hpp"
#include "../../logger.hpp"

#ifdef __APPLE__
// Generated Header for Swift library
#include "coreml_backend_swift/coreml_backend_swift.h" 
using namespace CoreMLSwift;
#endif


#ifdef __APPLE__
/// @brief CoreML backend for neural networks. Just a wrapper for Swift class.
class NNCoreML: public NNRuntime {
public:
    NNCoreML(): coreml(createCoreMLBackend()) {}

    int loadModel(const std::string& path) {
        return coreml.loadModel(path);
    }

    int run(float32_t* outputBuffer, cv::Mat& input) {
        cv::Mat imageARGB;
        cv::cvtColor(input, imageARGB, cv::COLOR_BGR2BGRA);
        int width = imageARGB.cols;
        int height = imageARGB.rows;
        CVPixelBufferWrapper inputWrapper = CVPixelBufferWrapper::init(width,height,imageARGB.data);
        return coreml.run(inputWrapper, outputBuffer);
    }

    cv::Size getInputSize() {
        return cv::Size(coreml.getInputWidth(), coreml.getInputHeight());
    }
private:
    CoreMLBackend coreml;
};
#else
/// @brief Stub for non Apple devices
class NNCoreML: public NNRuntime {
public:
    NNCoreML() {}

    int loadONNX(const std::string& path) {
        Logger::error("CoreML backend is not supported on this platform");
        return -1;
    }

    int run(float32_t* outputBuffer, cv::Mat& input) {
        Logger::error("CoreML backend is not supported on this platform");
        return -1;
    }

    cv::Size getInputSize() {
        Logger::error("CoreML backend is not supported on this platform");
        return cv::Size(0,0);
    }
};
#endif