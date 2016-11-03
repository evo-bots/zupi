#pragma once

#include <string>
#include "vision.h"

class FaceDetector : public ::vision::Detector {
public:
    FaceDetector(const ::std::string& dir, bool lbp, double scaleFactor, int minNeighbors, int minSize);

    size_t detect(const ::cv::Mat& image, ::vision::DetectedObjectList& objects);

private:
    ::vision::ClassifyModel m_frontalface;
    ::vision::ClassifyModel m_eyes;
};
