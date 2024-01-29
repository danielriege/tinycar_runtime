#pragma once

#include <opencv2/core.hpp>

#include "../../provider.hpp"
#include "tinycar.hpp"

///
/// \brief Just a wrapper for the Tinycar class to make it compatible with the Provider interface
class TinycarProvider : public Provider {
public:
    TinycarProvider(std::shared_ptr<Tinycar> tinycar) : tinycar(tinycar) {
        
    };

    int getImage(cv::Mat& out) {
        return tinycar->getImage(out);
    }

private:
    std::shared_ptr<Tinycar> tinycar;
};