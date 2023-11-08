#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "helpers.hpp"
#include "nv.hpp"
float maxVal = 255;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " IMAGEFILE" << std::endl;
        return -1;
    }

    // create window
    auto window = initGui("tinycar_esp_runtime");
    nv::DrawList::getInstance().loadState();

    
    // main loop
    while (!glfwWindowShouldClose(window)) {
        frameStart();

        bool reset = false;
        // add slider and button to config panel
        ImGui::Begin("Config");
        if (ImGui::SliderFloat("maxVal", &maxVal, 0, 255)) {
            std::cout << "maxVal changed to " << maxVal << std::endl;
        }
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