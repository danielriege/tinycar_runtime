#include "nv.hpp"

#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// #include "helpers.hpp"

#define DEBUG_RENDER 1

namespace nv {
DrawList& DrawList::getInstance() {
    static DrawList instance;
    return instance;
}
void DrawList::loadState(std::string path) {
    std::ifstream infile(path);

    std::string line;
    while (std::getline(infile, line)) {
        spaces[line.substr(2)].enabled = (line[0] == '1');
    }
}
void DrawList::saveState(std::string path) {
    std::ofstream outfile(path);
    for (auto& [k, v] : spaces) {
        // if (v.obs.size() == 0) continue;
        outfile << (v.enabled ? '1' : '0');
        outfile << " " << k << "\n";
    }
}
void putText(std::string ns, std::string text, ImVec2 anchor, ImU32 color, ImU32 colorbg) {
#if DEBUG_RENDER
    DrawList::Text t;
    t.color = color;
    t.colorbg = colorbg;
    t.anchor = anchor;
    t.text = text;
    DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Text>(t));
#endif
}

void line(std::string ns, ImVec2 start, ImVec2 end, ImU32 color, float thickness) {
#if DEBUG_RENDER
    DrawList::Line l;
    l.color = color;
    l.thickness = thickness;
    l.start = start;
    l.end = end;
    DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
#endif
}

void arrow(std::string ns, ImVec2 start, ImVec2 end, ImU32 color, float thickness, float headlength) {
#if DEBUG_RENDER
    auto dir = end - start;
    dir /= sqrtf(dir.x * dir.x + dir.y * dir.y);
    cv::Point2f normal(dir.y, -dir.x);
    nv::line(ns, start, end, color, thickness);
    nv::line(ns, end - (dir - normal / 2) * headlength, end, color, thickness);
    nv::line(ns, end - (dir + normal / 2) * headlength, end, color, thickness);
#endif
}

void drawMarker(std::string ns, ImVec2 pos, ImU32 color, int markerType, float size, float thickness) {
#if DEBUG_RENDER
    ImVec2 offx(size, 0);
    ImVec2 offy(0, size);
    DrawList::Line l;
    l.color = color;
    l.thickness = thickness;
    switch (markerType) {
        default:  // cross
            l.start = pos - offx;
            l.end = pos + offx;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offy;
            l.end = pos + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 1:  // tilted cross
            l.start = pos - offx - offy;
            l.end = pos + offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offx - offy;
            l.end = pos - offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 2:  // star
            l.start = pos - offx;
            l.end = pos + offx;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offy;
            l.end = pos + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offx - offy;
            l.end = pos + offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offx - offy;
            l.end = pos - offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 3:  // diamond
            l.start = pos - offx;
            l.end = pos + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offy;
            l.end = pos + offx;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offx;
            l.end = pos - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offy;
            l.end = pos - offx;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 4:  // square
            l.start = pos - offx - offy;
            l.end = pos - offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offx + offy;
            l.end = pos + offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offx + offy;
            l.end = pos + offx - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offx - offy;
            l.end = pos - offx - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 5:  // triangle up
            l.start = pos - offy;
            l.end = pos - offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offy;
            l.end = pos + offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offx + offy;
            l.end = pos + offx + offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
        case 6:  // triangle down
            l.start = pos + offy;
            l.end = pos - offx - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos + offy;
            l.end = pos + offx - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            l.start = pos - offx - offy;
            l.end = pos + offx - offy;
            DrawList::getInstance().spaces[ns].obs.push_back(std::make_shared<DrawList::Line>(l));
            break;
    }
#endif
}
void clear() {
    for (auto& [k, v] : DrawList::getInstance().spaces) {
        v.obs.clear();
    }
}

void draw(ImVec2 origin, float zoom, std::string ns) {
    auto len = ns.size();
    for (auto& [k, v] : DrawList::getInstance().spaces) {
        if (v.enabled && k.substr(0, len) == ns)
            for (auto o : v.obs) {
                o->draw(origin, zoom);
            }
    }
}

void showSettings() {
    ImGuiTextBuffer label;
    ImGui::Begin("Config");

    std::string lastTreeNode = "";
    bool open = false;
    bool action = false;
    bool allaction = false;
    bool newState = false;

    ImGui::Text("all");
    ImGui::SameLine(ImGui::GetWindowWidth() - 84 - 20);
    if (ImGui::SmallButton("show")) {
        allaction = true;
        newState = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("hide")) {
        allaction = true;
        newState = false;
    }

    std::vector<std::string> stack;
    for (auto& [k, v] : DrawList::getInstance().spaces) {
        auto name = k;
        auto lastSep = name.find_last_of('.');
        std::string treeNode = "";
        if (lastSep < name.size()) {
            treeNode = name.substr(0, lastSep);
        }
        if (treeNode != lastTreeNode && treeNode != "") {
            action = false;
            if (open) ImGui::TreePop();
            ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
            open = ImGui::TreeNodeEx(treeNode.c_str(), ImGuiTreeNodeFlags_AllowOverlap);

            ImGui::SameLine(ImGui::GetWindowWidth() - 84 - 20);
            if (ImGui::SmallButton("show")) {
                action = true;
                newState = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("hide")) {
                action = true;
                newState = false;
            }
        }
        name = name.substr(lastSep + 1);
        lastTreeNode = treeNode;

        if (treeNode == "") {
            action = false;
            if (open) {
                ImGui::TreePop();
                open = false;
            }
        }
        if (action || allaction) v.enabled = newState;

        if (treeNode == "" || open) {
            label.clear();
            label.appendf("%s", name.c_str());
            ImGui::Checkbox(label.c_str(), &v.enabled);
        }
    }
    if (open) ImGui::TreePop();
    ImGui::End();
}

Images& Images::getInstance() {
    static Images instance;
    return instance;
}

void showInspector(std::string name, ImVec2 pos) {
    auto& l = Images::getInstance().list;
    auto im = l.find(name);
    if (im == l.end()) return;
    auto image = im->second.image;
    if (pos.x < 0 || pos.y < 0 || pos.x >= image.cols || pos.y >= image.rows) return;
    auto origin = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(origin, origin + ImVec2(196, 13), ImColor(0, 0, 0, 100), 4);
    if (image.channels() == 3 && image.depth() == CV_8U) {
        auto pix = image.at<cv::Vec3b>(pos.y, pos.x);
        ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
        ImGui::SameLine();
        ImGui::TextColored(ImColor(255, 80, 0), "%3d", pix[2]);
        ImGui::SameLine();
        ImGui::TextColored(ImColor(0, 255, 0), "%3d", pix[1]);
        ImGui::SameLine();
        ImGui::TextColored(ImColor(100, 100, 255), "%3d", pix[0]);
    } else if (image.channels() == 1) {
        if (image.depth() == CV_32F) {
            auto pix = image.at<float>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%10.2f", pix);
        }
        if (image.depth() == CV_64F) {
            auto pix = image.at<double>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%10.2f", pix);
        }
        if (image.depth() == CV_8U) {
            auto pix = image.at<uint8_t>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%3d", pix);
        }
        if (image.depth() == CV_8S) {
            auto pix = image.at<int8_t>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%4d", pix);
        }
        if (image.depth() == CV_16S) {
            auto pix = image.at<int16_t>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%6d", pix);
        }
        if (image.depth() == CV_16U) {
            auto pix = image.at<uint16_t>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%6d", pix);
        }
        if (image.depth() == CV_32S) {
            auto pix = image.at<int32_t>(pos.y, pos.x);
            ImGui::Text("x: %4d y: %4d", (int)pos.x, (int)pos.y);
            ImGui::SameLine();
            ImGui::Text("%10d", pix);
        }
    }
}

void showProfiler() {
    ImGui::Begin("Profiler");
    ImGui::SetWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    auto& m = ProfileContainer::getInstance();
    for (auto& [k, v] : m) {
        ImGui::Text("%s", k.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - (8 + 3) * 8);
        ImGui::Text("%8.2f ms", v.current * 1000);
    }
    ImGui::End();
}

void imshow(std::string name, cv::Mat image, double max_val, double min_val, int colormap) {
    auto& l = Images::getInstance().list;
    auto im = l.find(name);
    if (im == l.end()) {
        l[name];
        im = l.find(name);

        glGenTextures(1, &im->second.texId);
        glBindTexture(GL_TEXTURE_2D, im->second.texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
    glBindTexture(GL_TEXTURE_2D, im->second.texId);
    im->second.image = image;
    cv::Mat tmp = image;
    if (image.depth() != CV_8U) {
        tmp.convertTo(tmp, CV_8U, 255 / (max_val - min_val), -min_val * 255 / (max_val - min_val));
    }

    if (image.channels() == 1) {
        if (colormap >= 0) {
            cv::applyColorMap(tmp, tmp, colormap);
        } else {
            cv::cvtColor(tmp, tmp, cv::COLOR_GRAY2RGBA);
        }
    }
    // has to be separate check b/c colormap results in BGR image
    if (tmp.channels() == 3) {
        cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGBA);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp.cols, tmp.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp.data);
}

bool ScrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    bool hovered = false;
    bool held = false;
    ImGuiID id = window->GetID("##scrolldraggingoverlay");
    ImGui::KeepAliveID(id);
    ImGuiButtonFlags button_flags = 1 << mouse_button == 0;
    if (g.HoveredId == 0)  // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
        ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
    if (held && delta.x != 0.0f) ImGui::SetScrollX(window, window->Scroll.x + delta.x);
    if (held && delta.y != 0.0f) ImGui::SetScrollY(window, window->Scroll.y + delta.y);
    return held;
}
struct ZoomSettings {
    uint64_t frameCount = 0;
    float zoom = 0.0f;
    float new_zoom = 1.0f;
    bool zoom_changed = false;
    bool lastHeld = false;
    ImVec2 mouseStart;
};
float BeginZoomable(std::string ns, ImVec2 dummySize, ImVec2 contentSize, bool reset, ImVec2& mouse_on_content, bool& click) {
    static std::map<std::string, ZoomSettings> settings;
    auto& s = settings[ns];
    s.frameCount++;
    const float zoom_step = 1.1f;
    ImGui::BeginChild("Zoomable", ImVec2(0, 0), false,
                      (ImGuiWindowFlags_NoScrollWithMouse) | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    if ((s.zoom <= 0 || reset) && s.frameCount > 1) {
        auto qi = contentSize.y / contentSize.x;
        auto qa = dummySize.y / dummySize.x;
        if (qi < qa) {
            s.zoom = dummySize.x / contentSize.x;
            auto scaledContent = contentSize * s.zoom;
            ImVec2 scroll(dummySize.x, dummySize.y - (dummySize.y - scaledContent.y) * 0.5);
            ImGui::SetScrollX(scroll.x);
            ImGui::SetScrollY(scroll.y);
        } else {
            s.zoom = dummySize.y / contentSize.y;
            auto scaledContent = contentSize * s.zoom;
            ImVec2 scroll(dummySize.x - (dummySize.x - scaledContent.x) * 0.5, dummySize.y);
            ImGui::SetScrollX(scroll.x);
            ImGui::SetScrollY(scroll.y);
        }
    }

    auto m = ImGui::GetMousePos();
    auto w = ImGui::GetWindowPos();
    auto mouse_position_on_window = m - w;
    auto mouse_position_on_content = (ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY()) - dummySize + mouse_position_on_window) / (s.zoom);

    if (s.zoom_changed) {
        s.zoom = s.new_zoom;
        s.zoom_changed = false;
    } else {
        if (ImGui::IsWindowHovered()) {
            auto& io = ImGui::GetIO();
            if (io.MouseWheel != 0) {
                s.new_zoom = s.zoom * pow(zoom_step, io.MouseWheel);
                s.new_zoom = ImMin(ImMax(s.new_zoom, 0.1f), 100.f);
                s.zoom_changed = true;
            }
        }

        if (s.zoom_changed) {
            // generate dummy with new dimensions
            {
                auto origin = ImGui::GetCursorScreenPos();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Dummy(ImFloor(contentSize * s.new_zoom) + dummySize * 2);
                ImGui::PopStyleVar();
                ImGui::SetCursorScreenPos(origin);
            }

            auto new_mouse_position_on_content = mouse_position_on_content * (s.new_zoom);
            auto new_scroll = new_mouse_position_on_content - mouse_position_on_window + dummySize;

            // Set new scroll position for next to be used in next ImGui::BeginChild() call.
            ImGui::SetScrollX(new_scroll.x);
            ImGui::SetScrollY(new_scroll.y);
        }
    }
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    bool held = ScrollWhenDraggingOnVoid(mouse_delta * (-1), ImGuiMouseButton_Left);
    mouse_on_content = mouse_position_on_content;
    if (!s.lastHeld && held) s.mouseStart = mouse_position_on_window;
    bool newClickState = false;
    if (s.lastHeld && !held) {
        // check if we dragged
        auto diffPos = s.mouseStart - mouse_position_on_window;
        // only detect click on button release and no movement from mouse!
        if (diffPos.x == 0 && diffPos.y == 0) {
            newClickState = true;
        }
    }
    click = newClickState;
    s.lastHeld = held;
    return s.zoom;
}

void EndZoomable() { ImGui::EndChild(); }
bool renderImageviewers(ImVec2& contentPos, bool reset) {
    bool ret = false;
    auto& l = nv::Images::getInstance().list;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));  // Get item spacing out of the equation.
    std::map<std::string, std::string> multiSelected;
    // render all tabs for the images
    const std::string DEFAULT_VIEWER = "image viewer";
    for (auto& [k, v] : l) {
        auto pos = k.find(':');
        std::string imns = DEFAULT_VIEWER;
        std::string tabName = k;
        if (pos != std::string::npos) {
            imns = k.substr(0, pos);
            tabName = k.substr(pos + 1);
        }
        ImGui::Begin(imns.c_str());
        ImGui::SetWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::BeginTabBar("#images");
        if (ImGui::BeginTabItem(tabName.c_str())) {
            multiSelected[imns] = k;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        ImGui::End();
    }
    for (auto& [imns, selected] : multiSelected) {
        ImGui::Begin(imns.c_str());
        if (selected != "") {
            ImVec2 mouse_on_content;
            bool click;
            ImVec2 dummySize = ImGui::GetContentRegionAvail();
            // ImVec2 dummySize = ImVec2(l[selected].image.cols, l[selected].image.rows);
            auto zoom = BeginZoomable(imns, dummySize, ImVec2(l[selected].image.cols, l[selected].image.rows), reset, mouse_on_content, click);

            ImGuiTextBuffer label;
            auto imsize = ImVec2(ImFloor(l[selected].image.cols * zoom), ImFloor(l[selected].image.rows * zoom));
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Dummy(imsize + dummySize * 2);
            ImGui::SetCursorScreenPos(p + dummySize);
            auto origin = ImGui::GetCursorScreenPos();

            if (click) {
                ret = true;
                contentPos = mouse_on_content;
            }
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(l[selected].texId)), ImVec2(l[selected].image.cols, l[selected].image.rows) * zoom);
            if (imns == DEFAULT_VIEWER)
                nv::draw(origin, zoom);
            else
                nv::draw(origin, zoom, imns);

            ImGui::SetCursorScreenPos(p + ImVec2(ImGui::GetScrollX() + dummySize.x - 210, ImGui::GetScrollY()));
            nv::showInspector(selected, mouse_on_content);
            EndZoomable();
        }
        ImGui::End();
    }
    ImGui::PopStyleVar();
    return ret;
}
};  // namespace nv