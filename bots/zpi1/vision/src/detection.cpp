#include "detection.h"
#include "analyzer.h"
#include "face_detector.h"

using namespace std;
using namespace zupi;
using namespace vision;

Detection::Detection() {
}

void Detection::init(Application *app) {
    string modelsDir = app->exeDir() + "/../share/OpenCV/";
    m_detectors["face"] = new FaceDetector(modelsDir, false, 1.1, 3, 30);
    m_detectors["fast"] = new FaceDetector(modelsDir, true, 2, 1, 60);
    m_detectors["analyzer"] = new Analyzer(modelsDir);
}

Detector* Detection::get(const string &name) const {
    auto it = m_detectors.find(name);
    return it == m_detectors.end() ? NULL : it->second;
}

vector<string> Detection::names() const {
    vector<string> names;
    for (auto& entry : m_detectors) {
        names.push_back(entry.first);
    }
    return names;
}
