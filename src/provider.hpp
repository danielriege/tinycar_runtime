#pragma once

#include <opencv2/core.hpp>

class Provider {
public:
    virtual ~Provider() {}

    virtual int getImage(cv::Mat&) = 0;
};
