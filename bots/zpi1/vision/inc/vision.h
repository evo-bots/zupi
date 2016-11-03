#pragma once

#include <string>
#include <list>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "json.hpp"
#include "zupi/common.h"

namespace vision {

struct DetectedObject {
    ::cv::Rect rc;
    ::std::string type;

    nlohmann::json json() const;
    operator nlohmann::json() const { return json(); }
};

struct DetectedObjectList : public ::std::list<DetectedObject> {
    nlohmann::json json() const;
    operator nlohmann::json() const { return json(); }
};

struct Detector : ::zupi::Interface {
    virtual size_t detect(const ::cv::Mat &image, DetectedObjectList &objects) = 0;
};

class Model : public Detector {
public:
    Model() {}

    const ::std::list<Model*>& children() const { return m_children; }
    void add(Model* model);

private:
    ::std::list<Model*> m_children;
};

class ClassifyModel : public Model {
public:
    ClassifyModel(const ::std::string &type, const ::std::string &fn,
        double scaleFactor = 1.1, int minNeighbors = 3,
        int flags = 0);//CV_HAAR_DO_CANNY_PRUNING | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH);

    double scaleFactor() const { return m_scaleFactor; }
    ClassifyModel& scaleFactor(double f) { m_scaleFactor = f; return *this; }
    int minNeighbors() const { return m_minNeighbors; }
    ClassifyModel& minNeighbors(int n) { m_minNeighbors = n; return *this; }
    int flags() const { return m_flags; }
    ClassifyModel& flags(int f) { m_flags = f; return *this; }

    const ::cv::Size& minSize() const { return m_minSize; }
    ClassifyModel& minSize(const ::cv::Size& size) { m_minSize = size; return *this; }
    const ::cv::Size& maxSize() const { return m_maxSize; }
    ClassifyModel& maxSize(const ::cv::Size& size) { m_maxSize = size; return *this; }

    size_t detect(const ::cv::Mat &image, DetectedObjectList &objects);

private:
    ::std::string m_type;
    ::cv::CascadeClassifier m_classifier;
    double m_scaleFactor;
    int m_minNeighbors;
    int m_flags;
    ::cv::Size m_minSize;
    ::cv::Size m_maxSize;
};

class AltModel : public Model {
public:
    AltModel() {}
    size_t detect(const ::cv::Mat &image, DetectedObjectList &objects);
};

class MultiModel : public Model {
public:
    MultiModel() {}
    size_t detect(const ::cv::Mat &image, DetectedObjectList &objects);
};

struct DetectResult {
    ::cv::Size size;
    DetectedObjectList objects;

    DetectResult() { }
    DetectResult(Detector*, const ::cv::Mat&);

    nlohmann::json json() const;
    operator nlohmann::json() const { return json(); }

    bool empty() const { return objects.empty(); }
};

}
