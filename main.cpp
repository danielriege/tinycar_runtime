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

    // read some image
    cv::Mat color = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (color.data == 0) {
        std::cout << "could not load image!" << std::endl;
        exit(-1);
    }

    // you have to init gui before using imshow (it needs opengl context running)
    nv::imshow("color", color);

    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);
    nv::imshow("gray", gray);

    cv::Mat grayFloat;
    gray.convertTo(grayFloat, CV_32F);
    nv::imshow("colormap", grayFloat, maxVal, 0, cv::COLORMAP_JET);

    std::vector<cv::Mat> channels;
    // using a different viewer for the channels
    {
        PROFILE_SCOPE("split+show")
        cv::split(color, channels);
        nv::imshow("channels:B", channels[0]);
        nv::imshow("channels:G", channels[1]);
        nv::imshow("channels:R", channels[2]);
    }

    // annotate images (working with floatingpoint values)
    // coordinates are pixel center
    {
        PROFILE_SCOPE("annotations")
        nv::drawMarker("markers.cross", cv::Point2f(0, 0), ImColor(255, 0, 0), cv::MARKER_CROSS, 7, 0.1);
        for (int i = 0; i < 20; ++i)
            nv::drawMarker("markers.tiltedCross", cv::Point2f(5 + 2 * i, 5 + 2 * i), ImColor(255, 255, 0), cv::MARKER_TILTED_CROSS, 0.5, 0.1);
        nv::line("lines.1px", {50, 50}, {150, 50}, ImColor(0, 0, 255), 1);
        nv::line("lines.thin", {50, 53.5}, {150, 53.5}, ImColor(0, 255, 0), 0.01);
        nv::arrow("arrow", {50, 73.5}, {150, 73.5}, ImColor(0, 255, 0), 1, 20);
        double minval, maxval;
        cv::Point2i minIdx, maxIdx;
        cv::minMaxLoc(channels[0], &minval, &maxval, &minIdx, &maxIdx);
        nv::drawMarker("channels:Bmax", cv::Point2f(maxIdx.x, maxIdx.y), ImColor(0, 0, 255), cv::MARKER_CROSS, 7, 0.1);
        nv::putText("channels:Bmax", "blue max (" + std::to_string((int)maxval) + ") at (" + std::to_string(maxIdx.x) + ", " + std::to_string(maxIdx.y) + ")",
                    cv::Point2f(maxIdx.x, maxIdx.y), ImColor(0, 0, 255), ImColor(0, 0, 0, 100));

        cv::minMaxLoc(channels[1], &minval, &maxval, &minIdx, &maxIdx);
        nv::drawMarker("channels:Gmax", cv::Point2f(maxIdx.x, maxIdx.y), ImColor(0, 255, 0), cv::MARKER_CROSS, 7, 0.1);
        nv::putText("channels:Gmax", "green max (" + std::to_string((int)maxval) + ") at (" + std::to_string(maxIdx.x) + ", " + std::to_string(maxIdx.y) + ")",
                    cv::Point2f(maxIdx.x, maxIdx.y), ImColor(0, 255, 0), ImColor(0, 0, 0, 100));

        cv::minMaxLoc(channels[2], &minval, &maxval, &minIdx, &maxIdx);
        nv::drawMarker("channels:Rmax", cv::Point2f(maxIdx.x, maxIdx.y), ImColor(255, 0, 0), cv::MARKER_CROSS, 7, 0.1);
        nv::putText("channels:Rmax", "red max (" + std::to_string((int)maxval) + ") at (" + std::to_string(maxIdx.x) + ", " + std::to_string(maxIdx.y) + ")",
                    cv::Point2f(maxIdx.x, maxIdx.y), ImColor(255, 0, 0), ImColor(0, 0, 0, 100));
    }
    // main loop
    while (!glfwWindowShouldClose(window)) {
        frameStart();

        bool reset = false;

        // add slider and button to config panel
        ImGui::Begin("Config");
        if (ImGui::SliderFloat("maxVal", &maxVal, 0, 255)) {
            nv::imshow("colormap", grayFloat, maxVal, 0, cv::COLORMAP_JET);
        }
        if (ImGui::Button("reset zoom")) {
            reset = true;
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