#pragma once

#include <opencv2/core.hpp>
#include <chrono>
#include "../../provider.hpp"
#include "../../logger.hpp"

class FileVideoProvider : public Provider {
public:
    FileVideoProvider(const std::string& filename, bool loop = true)
        : filename(filename), loop(loop), lastFrameTime(std::chrono::steady_clock::now()) {
        cap.open(filename);
        if (cap.isOpened()) {
            fps = cap.get(cv::CAP_PROP_FPS);
            frameTime = std::chrono::duration<double>(1.0 / fps);
            playing = true;
            Logger::info("Opened video file: " + filename + " with " + std::to_string(fps) + " fps");
        } else {
            Logger::error("Could not open video file: " + filename);
        }
    }

    int getImage(cv::Mat& out) {
        auto now = std::chrono::steady_clock::now();
        if (now - lastFrameTime >= frameTime && playing) {
            if (!cap.read(out)) {
                if (loop) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                    cap.read(out);
                } else {
                    Logger::info("Reached end of video");
                    return false;
                }
            }
            lastFrameTime = now;
            return true;
        }
        return false;
    }

    void gotoFrame(int frame) {
        cap.set(cv::CAP_PROP_POS_FRAMES, frame);
    }

    /// @brief Returns the video length in frames
    /// @return video length in frames
    int getVideoLength() {
        return cap.get(cv::CAP_PROP_FRAME_COUNT);
    }

    void resume() {
        if (!playing) {
            playing = true;
        }
    }

    void stop() {
        if (playing) {
            playing = false;
        }
    }

    bool isPlaying() {
        return playing;
    }

private:
    std::string filename;
    bool loop;
    cv::VideoCapture cap;
    double fps;
    std::chrono::steady_clock::time_point lastFrameTime;
    std::chrono::duration<double> frameTime;
    bool playing;
};