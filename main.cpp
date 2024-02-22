#include <iostream>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "helpers.hpp"
#include "nv.hpp" // also includes imgui

#include "logger.hpp"
#include "recorder.hpp"

#include "provider.hpp"
#include "backends/provider/file_image_provider.hpp"
#include "backends/provider/file_video_provider.hpp"
#include "backends/provider/tinycar_provider.hpp"

#include "nn_runtime.hpp"
#include "backends/nn/nn_coreml.hpp"

#include "viewcontroller/tinycar_viewcontroller.hpp"

bool getEnv(const std::string& key) {
    const char* value = std::getenv(key.c_str());
    return value != nullptr && std::string(value) == "1";
}

enum class ProviderType {
    IMAGE,
    VIDEO,
    TINYCAR
};
ProviderType providerType;

cv::Scalar channel_color_lookup[] = {
    cv::Scalar(0x01, 0x01, 0xFF),  // outer
    cv::Scalar(0x01, 0x9F, 0xFF),  // middle
    cv::Scalar(0x01, 0x5F, 0x33),  // guide
    cv::Scalar(0x01, 0xEB, 0xFF),  // solid
    cv::Scalar(0xFF, 0x24, 0x01),  // hold
    cv::Scalar(0xFF, 0xAC, 0x66),  // zebra
    cv::Scalar(0x01, 0x01, 0x01)   // last color
};

typedef struct {
    cv::Size inputSize;
    float* nnOutputRawBuffer; // pointer to a possibly used buffer
    std::vector<cv::Mat> outputMats; // vector for output images
    uint8_t nOutputMats; // number of elements in outputMats array
} nn_config_t;

///////// PROPERTIES

std::shared_ptr<Provider> imageProvider;
std::shared_ptr<NNRuntime> nnRuntime;
std::shared_ptr<nn_config_t> nnConfig;
std::shared_ptr<Recorder> recorder;
std::shared_ptr<Tinycar> tinycar;


bool doLaneDetection = false;

// view controller
std::unique_ptr<TinycarViewController> tinycarViewController;

char* getCmdOption(char ** begin, char ** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int prepareNNRuntime(std::shared_ptr<nn_config_t> config) {
    cv::Size inputSize = nnRuntime->getInputSize();
    if (inputSize.width <= 0 || inputSize.height <= 0) {
        Logger::error("Invalid input size width: " + std::to_string(inputSize.width) + " height: " + std::to_string(inputSize.height));
        return EXIT_FAILURE;
    }
    Logger::info("Input size: " + std::to_string(inputSize.width) + "x" + std::to_string(inputSize.height));
    config->inputSize = inputSize;
    config->nOutputMats = 7;

    // allocate output buffer
    config->nnOutputRawBuffer = new float[config->nOutputMats * inputSize.width * inputSize.height];
    config->outputMats = std::vector<cv::Mat>(config->nOutputMats);
    for (int i = 0; i < 7; i++) {
        config->outputMats[i] = cv::Mat(inputSize, CV_8UC1);
    }
    return 0;
}

void setupViewController() {
    tinycarViewController = std::make_unique<TinycarViewController>(tinycar);
}

void parseEnvVariables() {
     // setting NN runtime
    if (getEnv("COREML")) {
        Logger::info("Using CoreML as NN runtime");
        nnRuntime = std::make_shared<NNCoreML>();
    } else {
        // TODO: add default NN runtime (probably CPU?)
        Logger::info("Using CoreML as NN runtime");
        nnRuntime = std::make_shared<NNCoreML>();
    }
}

int parseProcessArguments(int argc, char** argv) {
    // print help
    if (cmdOptionExists(argv, argv + argc, "-h") || cmdOptionExists(argv, argv + argc, "--help")) {
        std::cout << "Usage: " << argv[0] << "-t <hostname/ip> -m <model> -f <file>" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -f <file>           Path to image or video file (offline testing)" << std::endl;
        std::cout << "  -m <model>          Path to model file" << std::endl;
        std::cout << "  -t <hostname/ip>    Hostname of tinycar" << std::endl;
        std::cout << "  -h                  Show this help" << std::endl;
        return EXIT_FAILURE;
    }
    
    ///// set provider
    // parse image or video file (offline testing)
    char* file_path = getCmdOption(argv, argv + argc, "-f");
    if (file_path) {
        std::string file_path_str = std::string(file_path);
        std::string extension = file_path_str.substr(file_path_str.find_last_of(".") + 1);
        if (extension == "mp4") {
            Logger::info("Using Video as provider backend");
            imageProvider = std::make_shared<FileVideoProvider>(file_path_str);
            providerType = ProviderType::VIDEO;
        } else {
            Logger::info("Using Image as provider backend");
            imageProvider = std::make_shared<FileImageProvider>(file_path_str);
            providerType = ProviderType::IMAGE;
        }
    } else {
        // set tinycar provider if no file is provided
        Logger::info("Using Tinycar as provider backend");
        char* host = getCmdOption(argv, argv + argc, "-t");
        if (host) {
            tinycar = std::make_shared<Tinycar>(std::string(host));
            imageProvider = std::make_shared<TinycarProvider>(tinycar);
            providerType = ProviderType::TINYCAR;
        } else {
            tinycar = std::make_shared<Tinycar>("localhost");
            imageProvider = std::make_shared<TinycarProvider>(tinycar);
            providerType = ProviderType::TINYCAR;
        }
    }

    // parse model file
    char* model_path = getCmdOption(argv, argv + argc, "-m");
    if (model_path) {
        std::string model_file = std::string(model_path);
        if (nnRuntime->loadModel(model_file) < 0) {
            Logger::error("Could not load model: " + std::string(model_file));
            return EXIT_FAILURE;
        }
        if (prepareNNRuntime(nnConfig) != 0) {
            return EXIT_FAILURE;
        }
        doLaneDetection = true;
    }

    return 0;
}

int main(int argc, char** argv) {

    // create window
    auto window = initGui("tinycar_esp_runtime");
    nv::DrawList::getInstance().loadState();

    recorder = std::make_shared<Recorder>();

    nnConfig = std::make_shared<nn_config_t>();
    parseEnvVariables();
    if (parseProcessArguments(argc, argv) != 0) {
        return EXIT_FAILURE;
    }
    if (providerType == ProviderType::TINYCAR) {
        setupViewController();
    }
    // only used for video provider
    int currentPlaybackSliderPosition = 0;

    // main loop
    // Loop sections: Tinycar Control, Video Playback Control, NN Execution
    while (!glfwWindowShouldClose(window)) {
        frameStart();
        //////////  Tinycar Control
        // show Tinycar window if tinycar provider is used
        if (providerType == ProviderType::TINYCAR) {
            // read gamepad input
            tinycarViewController->readGamepadInput();
            tinycarViewController->sendControlMessage();
            // show tinycar view controller
            tinycarViewController->show();
        }

        // show playback control if video provider is used
        if (providerType == ProviderType::VIDEO) {
            // static cast provider to video provider
            auto videoProvider = std::static_pointer_cast<FileVideoProvider>(imageProvider);
            int frameCount = videoProvider->getVideoLength();
            ImGui::Begin("Playback Control");
            if (ImGui::Button("Play")) {
                videoProvider->resume();
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause")) {
                videoProvider->stop();
            }
            ImGui::SameLine();
            if (ImGui::SliderInt("Timeline", &currentPlaybackSliderPosition, 0, frameCount)) {
                videoProvider->gotoFrame(currentPlaybackSliderPosition);
            }
            ImGui::End();
        }

        // show recorder
        ImGui::Begin("Recorder");
        if (recorder->isRecordingVideo()) {
            ImGui::Text("Recording...");
            if (ImGui::Button("Stop Recording")) {
                recorder->stopRecord();
            }
        } else {
            ImGui::Text("Not Recording");
            if (ImGui::Button("Start Recording")) {
                recorder->startRecord(imageProvider->getFPS());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Image")) {
            recorder->takeImage();
        }
        ImGui::End();

        // show next frame if available
        cv::Mat image;
        if (imageProvider->getImage(image)) {
            currentPlaybackSliderPosition++;
            recorder->provideFrame(image);
            nv::imshow("tinycar_image:input", image);

            // do some preprocessing
            if (doLaneDetection) {
                cv::Mat input;
                {
                    PROFILE_SCOPE("preprocessing");
                    // crop image to use only lower half
                    input = image(cv::Rect(0, image.rows / 2, image.cols, image.rows / 2));
                    // resize image to input size
                    cv::resize(input, input, nnConfig->inputSize);
                    nv::imshow("nn:preprocessed", input);
                }

                {
                    // PROFILE_SCOPE("inference");
                    // if (nnRuntime->run(nnConfig->nnOutputRawBuffer, input) < 0) {
                    //     Logger::error("Could not run model");
                    //     return EXIT_FAILURE;
                    // }
                }

                {
                    PROFILE_SCOPE("raw output split");
                    // debug output window, for each channel one cv::Mat
                    for (int c = 0; c < 7; c++) {
                        int y = 0;
                        for (int i = c; i < 7 * nnConfig->inputSize.width * nnConfig->inputSize.height; i = i + 7) {
                            //outputMats[c].data[y] = outputRawBuffer[i];
                            nnConfig->outputMats[c].data[y] = nnConfig->nnOutputRawBuffer[i] * 255;
                            y++;
                        }
                        nv::imshow("nn_raw_output:ch" + std::to_string(c), nnConfig->outputMats[c]);
                    }
                }

                {
                    PROFILE_SCOPE("output merge");
                    cv::Mat combined(nnConfig->inputSize, CV_8UC3, cv::Scalar(0, 0, 0));  // Initialize to black

                    for (int c = 0; c < 7; c++) {  // Change this to the number of channels you have
                        cv::Mat mask;  // Threshold your mask here
                        cv::threshold(nnConfig->outputMats[c], mask, 150, 255, cv::THRESH_BINARY);
                        combined.setTo(channel_color_lookup[c], mask);  // Set the color of the pixels in the mask
                    }
                    
                    nv::imshow("nn:output", combined);
                }
            }
        }

        bool reset = false;
        // add slider and button to config panel
        ImGui::Begin("Config");
        if (ImGui::Button("reset zoom")) {
            reset = true;
            std::cout << "button pressed" << std::endl;
        }
        ImGui::End();

        // show annotation settings in config panel
        nv::showSettings();
        nv::showProfiler();
        ImVec2 contentPos;
        if (nv::renderImageviewers(contentPos, reset)) {
            std::cout << "click on: (" << contentPos.x << ", " << contentPos.y << ")" << std::endl;
        }

        frameEnd(window);
    }

    // save debug annotation states
    nv::DrawList::getInstance().saveState();
    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}