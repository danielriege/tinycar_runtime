#include <iostream>
#include <cstdlib>
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "helpers.hpp"
#include "nv.hpp"

#include "logger.hpp"

#include "provider.hpp"
#include "backends/provider/file_image_provider.hpp"
#include "backends/provider/file_video_provider.hpp"

#include "nn_runtime.hpp"
#include "backends/nn/nn_coreml.hpp"

std::shared_ptr<Provider> imageProvider;
std::shared_ptr<NNRuntime> nnRuntime;

bool getEnv(const std::string& key) {
    const char* value = std::getenv(key.c_str());
    return value != nullptr && std::string(value) == "1";
}

enum class ProviderType {
    IMAGE,
    VIDEO
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

int main(int argc, char** argv) {

    // create window
    auto window = initGui("tinycar_esp_runtime");
    nv::DrawList::getInstance().loadState();

    // setting provider
    if (argc == 3) {
        // check if file is image or video
        std::string filename = argv[2];
        std::string extension = filename.substr(filename.find_last_of(".") + 1);
        if (extension == "mp4") {
            Logger::info("Using Video as provider backend");
            imageProvider = std::make_shared<FileVideoProvider>(filename);
            providerType = ProviderType::VIDEO;
        } else {
            Logger::info("Using Image as provider backend");
            imageProvider = std::make_shared<FileImageProvider>(filename);
            providerType = ProviderType::IMAGE;
        }
    } else {

    }

    // setting NN runtime
    if (getEnv("COREML")) {
        Logger::info("Using CoreML as NN runtime");
        nnRuntime = std::make_shared<NNCoreML>();
    } else {
        // TODO: add default NN runtime (probably CPU?)
        Logger::info("Using CoreML as NN runtime");
        nnRuntime = std::make_shared<NNCoreML>();
    }

    std::string modelPath = argv[1];
    {
        PROFILE_SCOPE("model load/compile");
        if (nnRuntime->loadModel(modelPath) < 0) {
            Logger::error("Could not load model: " + modelPath);
            return EXIT_FAILURE;
        }
    }
    cv::Size inputSize = nnRuntime->getInputSize();
    if (inputSize.width <= 0 || inputSize.height <= 0) {
        Logger::error("Invalid input size width: " + std::to_string(inputSize.width) + " height: " + std::to_string(inputSize.height) + " for model: " + modelPath);
        return EXIT_FAILURE;
    }
    Logger::info("Input size: " + std::to_string(inputSize.width) + "x" + std::to_string(inputSize.height));

    // allocate output buffer
    float32_t* outputRawBuffer = new float32_t[7 * inputSize.width * inputSize.height];
    cv::Mat outputMats[7];
    for (int i = 0; i < 7; i++) {
        outputMats[i] = cv::Mat(inputSize, CV_8UC1);
    }
    // only used for video provider
    int currentPlaybackSliderPosition = 0;

    // main loop
    while (!glfwWindowShouldClose(window)) {
        frameStart();

        // show playback control if video provider is used
        if (providerType == ProviderType::VIDEO) {
            // static cast provider to video provider
            auto videoProvider = std::static_pointer_cast<FileVideoProvider>(imageProvider);
            int frameCount = videoProvider->getVideoLength();
            ImGui::Begin("Playback Control");
            if (ImGui::Button("Play")) {
                videoProvider->resume();
            }
            if (ImGui::Button("Pause")) {
                videoProvider->stop();
            }
            if (ImGui::SliderInt("Timeline", &currentPlaybackSliderPosition, 0, frameCount)) {
                videoProvider->gotoFrame(currentPlaybackSliderPosition);
            }
            ImGui::End();
        }

        // show next frame if available
        cv::Mat image;
        if (imageProvider->getImage(image)) {
            currentPlaybackSliderPosition++;
            nv::imshow("tinycar_image:input", image);

            // do some preprocessing
            cv::Mat input;
            {
                PROFILE_SCOPE("preprocessing");
                // crop image to use only lower half
                input = image(cv::Rect(0, image.rows / 2, image.cols, image.rows / 2));
                // resize image to input size
                cv::resize(input, input, inputSize);
                nv::imshow("nn:preprocessed", input);
            }

            {
                PROFILE_SCOPE("inference");
                if (nnRuntime->run(outputRawBuffer, input) < 0) {
                    Logger::error("Could not run model");
                    return EXIT_FAILURE;
                }
            }

            {
                PROFILE_SCOPE("raw output split");
                // debug output window, for each channel one cv::Mat
                for (int c = 0; c < 7; c++) {
                    int y = 0;
                    for (int i = c; i < 7 * inputSize.width * inputSize.height; i = i + 7) {
                        //outputMats[c].data[y] = outputRawBuffer[i];
                        outputMats[c].data[y] = outputRawBuffer[i] * 255;
                        y++;
                    }
                    nv::imshow("nn_raw_output:ch" + std::to_string(c), outputMats[c]);
                }
            }

            {
                PROFILE_SCOPE("output merge");
                cv::Mat combined(inputSize, CV_8UC3, cv::Scalar(0, 0, 0));  // Initialize to black

                for (int c = 0; c < 7; c++) {  // Change this to the number of channels you have
                    cv::Mat mask;  // Threshold your mask here
                    cv::threshold(outputMats[c], mask, 150, 255, cv::THRESH_BINARY);
                    combined.setTo(channel_color_lookup[c], mask);  // Set the color of the pixels in the mask
                }
                
                nv::imshow("nn:output", combined);
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