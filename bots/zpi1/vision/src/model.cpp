#include <vector>
#include "vision.h"

namespace vision {

using namespace std;
using namespace cv;

nlohmann::json DetectedObject::json() const {
    return nlohmann::json({
        {"type", type},
        {"range", { {"x", rc.x}, {"y", rc.y}, {"w", rc.width}, {"h", rc.height} } }
    });
}

nlohmann::json DetectedObjectList::json() const {
    nlohmann::json j;
    for (auto& obj : *this) {
        j.push_back(obj);
    }
    return j;
}

DetectResult::DetectResult(Detector *detector, const Mat& image) {
    detector->detect(image, objects);
    size.width = image.cols;
    size.height = image.rows;
}

nlohmann::json DetectResult::json() const {
    return nlohmann::json({
        {"size", { {"w", size.width}, {"h", size.height} }},
        {"objects", objects},
    });
}

void Model::add(Model *child) {
    m_children.push_back(child);
}

ClassifyModel::ClassifyModel(const string& type, const string& fn,
    double scaleFactor, int minNeighbors, int flags)
: m_type(type),
  m_classifier(fn),
  m_scaleFactor(scaleFactor),
  m_minNeighbors(minNeighbors),
  m_flags(flags) {
}

size_t ClassifyModel::detect(const Mat& image, DetectedObjectList& objects) {
    vector<Rect> rects;
    m_classifier.detectMultiScale(image, rects, m_scaleFactor, m_minNeighbors, m_flags, m_minSize, m_maxSize);
    size_t count = 0;
    for (auto& r : rects) {
        objects.push_back({r, m_type});
        count ++;
        auto subImg = image(r);
        for (auto child : children()) {
            DetectedObjectList objs;
            count += child->detect(image, objs);
            for (auto& obj : objs) {
                obj.rc.x += r.x;
                obj.rc.y += r.y;
                objects.push_back(obj);
            }
        }
    }
    return count;
}

size_t AltModel::detect(const Mat& image, DetectedObjectList& objects) {
    for (auto child : children()) {
        auto count = child->detect(image, objects);
        if (count > 0) {
            return count;
        }
    }
    return 0;
}

size_t MultiModel::detect(const Mat& image, DetectedObjectList& objects) {
    size_t count = 0;
    for (auto child : children()) {
        count += child->detect(image, objects);
    }
    return 0;
}

}
