#pragma once

#include <opencv2/core.hpp>
#define IM_VEC2_CLASS_EXTRA                                    \
    constexpr ImVec2(const cv::Point2f& p) : x(p.x), y(p.y) {} \
    operator cv::Point2f() const { return cv::Point2f(x, y); }
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

namespace nv {

class DrawList {
   public:
    struct DrawObject {
        virtual ~DrawObject() {}
        virtual void draw(ImVec2 origin, float zoom) = 0;
    };

    struct Line : public DrawObject {
        const ImVec2 halfpixel = ImVec2(0.5, 0.5);
        ImVec2 start, end;
        ImU32 color;
        float thickness;
        virtual void draw(ImVec2 origin, float zoom) override {
            ImGui::GetWindowDrawList()->AddLine(origin + (start + halfpixel) * zoom, origin + (end + halfpixel) * zoom, color, (zoom * thickness));
        }
    };
    struct Text : public DrawObject {
        ImVec2 anchor;
        ImU32 color;
        ImU32 colorbg;
        std::string text;
        virtual void draw(ImVec2 origin, float zoom) override {
            const ImVec2 halfpixel = ImVec2(0.5, 0.5);
            if (colorbg != 0)
                ImGui::GetWindowDrawList()->AddRectFilled(origin + (anchor + halfpixel) * zoom,
                                                          origin + (anchor + halfpixel) * zoom + ImVec2(7 * text.size() + 4, 13), colorbg, 4);
            ImGui::GetWindowDrawList()->AddText(origin + (anchor + halfpixel) * zoom + ImVec2(2, 0), color, text.c_str());
        }
    };

   private:
    struct DrawGroup {
        bool enabled = true;
        std::vector<std::shared_ptr<DrawObject>> obs;
    };

   public:
    std::map<std::string, DrawGroup> spaces;

    /**
     * @brief singleton getter for DrawList object
     *
     * @return DrawList& instance
     */
    static DrawList& getInstance();

    /**
     * @brief loads current activation state for annotations from file
     *
     * @param path path to state file
     */
    void loadState(std::string path = "nv.txt");
    /**
     * @brief saves current activation state for annotations
     *
     * @param path path to state file
     */
    void saveState(std::string path = "nv.txt");
};

class Images {
    struct Image {
        cv::Mat image;
        uint32_t texId;
    };

   public:
    static Images& getInstance();

    std::map<std::string, Image> list;
};

struct ProfileContainer {
    double current;

    static std::map<std::string, ProfileContainer>& getInstance() {
        static std::map<std::string, ProfileContainer> instance;
        return instance;
    }
};

class ScopeProfiler {
   public:
    ScopeProfiler(std::string name, bool multi = false) : name(name), multi(multi), startTime(std::chrono::steady_clock::now()) {}

    ~ScopeProfiler() {
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
        auto& m = ProfileContainer::getInstance();
        if (multi)
            m[name].current += elapsed.count();
        else
            m[name].current = elapsed.count();
    }

   private:
    std::string name;
    bool multi;
    std::chrono::steady_clock::time_point startTime;
};

/**
 * @brief creates a text annotation
 *
 * @param ns annotation namespace
 * @param text text to render
 * @param anchor top left coordinate to anchor text to
 * @param color text color
 * @param colorbg background color
 */
void putText(std::string ns, std::string text, ImVec2 anchor, ImU32 color, ImU32 colorbg = ImColor(0, 0, 0, 0));

/**
 * @brief creates a line annotation
 *
 * @param ns annotation namespace
 * @param start start point
 * @param end end point
 * @param color color
 * @param thickness thickness in image pixels (scales with zoom)
 */
void line(std::string ns, ImVec2 start, ImVec2 end, ImU32 color, float thickness = 1);

/**
 * @brief creates an arrow annotation
 *
 * @param ns annotation namespace
 * @param start start point
 * @param end end point
 * @param color color
 * @param thickness thickness in image pixels (scales with zoom)
 * @param headlength length of arrow head in image pixels
 */
void arrow(std::string ns, ImVec2 start, ImVec2 end, ImU32 color, float thickness = 1, float headlength = 3);

/**
 * @brief creates a marker annotation
 *
 * @param ns annotation namespace
 * @param pos position of marker
 * @param color color of marker
 * @param markerType marker type like opencv (cv::MARKER_*)
 * @param size marker size in image pixels
 * @param thickness marker line thickness in image pixels
 */
void drawMarker(std::string ns, ImVec2 pos, ImU32 color, int markerType, float size = 7, float thickness = 1);

/**
 * @brief clears all annotations
 */
void clear();

/**
 * @brief draws annotations in current context
 *
 * @param origin zero coordinate for annotations
 * @param zoom zoom scaling
 * @param ns namespace prefix to select annotations for different windows
 */
void draw(ImVec2 origin, float zoom, std::string ns = "");

/**
 * @brief shows annotation tree and its checkboxes to (de-)activate drawing
 */
void showSettings();

/**
 * @brief renders image like cv::imshow, but with more options
 *
 * @param name image name: can be prefixed with window name like "window:image"
 * @param image image to render
 * @param max_val applies to non CV_8U images only
 * @param min_val applies to non CV_8U images only
 * @param colormap applies to non CV_8U images only
 */
void imshow(std::string name, cv::Mat image, double max_val = 1.0, double min_val = 0.0, int colormap = -1);

/**
 * @brief shows image information at pos (coordinates + color/value)
 *
 * @param name image name
 * @param pos image coordinates
 */
void showInspector(std::string name, ImVec2 pos);

/**
 * @brief shows a profiler window with measured times
 */
void showProfiler();

/**
 * @brief helper function to render all image viewers
 *
 * @param contentPos position on content when click happens
 * @param reset resets zoom and scroll
 * @return true click happend
 * @return false no click happend
 */
bool renderImageviewers(ImVec2& contentPos, bool reset);

#define PROFILE_SCOPE(...) auto PROFILE_COOKIE = nv::ScopeProfiler(__VA_ARGS__);
#define PROFILE_SCOPE_RESET(x) nv::ProfileContainer::getInstance()[x].current = 0;

};  // namespace nv