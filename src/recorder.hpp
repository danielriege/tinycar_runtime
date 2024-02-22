#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <opencv2/core.hpp>

class Recorder {
private:
    bool isRecording;
    cv::VideoWriter videoWriter;
    cv::Mat lastFrame;
    int videoCounter;
    int imageCounter;

public:
    Recorder() : isRecording(false), videoCounter(0), imageCounter(0) {}

    void startRecord(double fps) {
        isRecording = true;
        std::stringstream ss;
        ss << "recording_" << std::setw(5) << std::setfill('0') << videoCounter++ << ".mp4";
        std::string filename = ss.str();
        videoWriter.open(filename, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(lastFrame.cols, lastFrame.rows));
    }

    void stopRecord() {
        isRecording = false;
        videoWriter.release();
    }

    void takeImage() {
        std::stringstream ss;
        ss << "image_" << std::setw(4) << std::setfill('0') << imageCounter++ << ".png";
        std::string filename = ss.str();
        cv::imwrite(filename, lastFrame);
    }

    void provideFrame(const cv::Mat& frame) {
        lastFrame = frame.clone();
        if (isRecording) {
            videoWriter.write(frame);
        }
    }

    bool isRecordingVideo() {
        return isRecording;
    }
};


