#pragma once

#include <string>
#include "vision.h"

class Analyzer : public ::vision::Detector {
public:
    Analyzer(const ::std::string& dir);

    size_t detect(const ::cv::Mat& image, ::vision::DetectedObjectList& objects);

private:
    ::vision::MultiModel m_models;
    ::vision::AltModel m_faces;
    ::vision::AltModel m_bodies;
    ::vision::ClassifyModel m_smile;
    ::vision::ClassifyModel m_face_def;
    ::vision::ClassifyModel m_face_alt;
    ::vision::ClassifyModel m_face_alt2;
    ::vision::ClassifyModel m_face_pro;
    ::vision::ClassifyModel m_body_full;
    ::vision::ClassifyModel m_body_upper;
    ::vision::ClassifyModel m_body_lower;
};
