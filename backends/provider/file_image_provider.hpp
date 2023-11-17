#pragma once

#include <opencv2/core.hpp>

#include "../../provider.hpp"

class FileImageProvider : public Provider {
public:
    FileImageProvider(const std::string& filename): shown(false) {
        image = cv::imread(filename);
    }

    int getImage(cv::Mat& out) {
        if (!shown) {
            out = image;
            shown = true;
            return true;
        }
        return false;
    }
private:
    cv::Mat image;
    bool shown;
};