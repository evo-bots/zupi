#include <iostream>
#include "vision.h"
#include "zupi/app.h"

using namespace std;
using namespace cv;
using namespace zupi;
using namespace vision;

class Analyzer : public Detector {
public:
    Analyzer(const string& dir)
    : m_smile("smile", dir + "haarcascade_smile.xml"),
      m_face_def("face", dir + "haarcascade_frontalface_default.xml"),
      m_face_alt("face", dir + "haarcascade_frontalface_alt.xml"),
      m_face_alt2("face", dir + "haarcascade_frontalface_alt.xml"),
      m_face_pro("face", dir + "haarcascade_profileface.xml"),
      m_body_full("body", dir + "haarcascade_fullbody.xml"),
      m_body_upper("body", dir + "haarcascade_upperbody.xml"),
      m_body_lower("body", dir + "haarcascade_lowerbody.xml") {
        m_faces.add(&m_face_def); m_face_def.add(&m_smile);
        m_faces.add(&m_face_alt); m_face_alt.add(&m_smile);
        m_faces.add(&m_face_alt2); m_face_alt2.add(&m_smile);
        m_faces.add(&m_face_pro);
        m_bodies.add(&m_body_full);
        m_bodies.add(&m_body_upper);
        m_bodies.add(&m_body_lower);
        m_models.add(&m_faces);
        m_models.add(&m_bodies);
    }

    size_t detect(const Mat& image, DetectedObjectList& objects) {
        return m_models.detect(image, objects);
    }

    DetectResult detect(const Mat& image) {
        return move(DetectResult(this, image));
    }
private:
    MultiModel m_models;
    AltModel m_faces;
    AltModel m_bodies;
    ClassifyModel m_smile;
    ClassifyModel m_face_def;
    ClassifyModel m_face_alt;
    ClassifyModel m_face_alt2;
    ClassifyModel m_face_pro;
    ClassifyModel m_body_full;
    ClassifyModel m_body_upper;
    ClassifyModel m_body_lower;
};

class App : public zupi::Application {
public:
    App(int argc, char** argv)
        : zupi::Application(argc, argv) {
        if (m_argc < 2) {
            throw "Missing video source";
        }
        if (!m_vcap.open(m_argv[1])) {
            throw "Error opening video stream or file";
        }
    }

protected:
    virtual int run() {
        Analyzer analyzer(exeDir() + "/../share/OpenCV/haarcascades/");
        cv::Mat image;

        while (true) {
            if (!m_vcap.read(image)) {
                break;
            }
            auto result = analyzer.detect(image);
            if (!result.empty()) {
                cout << result.json() << endl;
                cout.flush();
            }
        }
        return 0;
    }

private:
    cv::VideoCapture m_vcap;
};

int main(int argc, char* argv[]) {
    return App(argc, argv).main();
}
