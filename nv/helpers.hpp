#pragma once
#include <opencv2/core.hpp>
#define IM_VEC2_CLASS_EXTRA                                    \
    constexpr ImVec2(const cv::Point2f& p) : x(p.x), y(p.y) {} \
    operator cv::Point2f() const { return cv::Point2f(x, y); }

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

#include <map>
static void glfw_error_callback(int error, const char* description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); }

static GLFWwindow* initGui(const char* windowTitle) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) exit(1);

        // Decide GL+GLSL versions
#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, windowTitle, nullptr, nullptr);
    if (window == nullptr) exit(1);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return window;
}

static void frameStart() {
    static bool initDone = false;
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();
    if (!initDone) {
        initDone = true;
        if (ImGui::GetCurrentContext()->SettingsIniData.size() == 0) {  // check if layout is stored in .ini
            printf("setting layout\n");
            // clear any previous layout
            ImGui::DockBuilderRemoveNodeChildNodes(dockspace_id);

            // set default layout
            ImGui::DockBuilderDockWindow("image viewer", dockspace_id);
            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
            ImGui::DockBuilderDockWindow("Config", dock_id_prop);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}

static void frameEnd(GLFWwindow* window) {
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}