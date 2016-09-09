#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "json.hpp"
#include "app.h"

using json = nlohmann::json;

struct Model {
    std::string type;
    cv::CascadeClassifier classifier;

    Model(const std::string &_type, const std::string &_fn)
        : type(_type), classifier(_fn) {
    }
};

struct Result {
    cv::Rect rect;
    std::string type;
};

class Detector {
public:
    Detector(const ::std::string &baseDir) {
        auto dir = baseDir + "/opencv/haarcascades/";
        models.push_back(new Model("face", dir + "haarcascade_frontalface_default.xml"));
        models.push_back(new Model("face", dir + "haarcascade_frontalface_alt.xml"));
        models.push_back(new Model("face", dir + "haarcascade_frontalface_alt2.xml"));
        models.push_back(new Model("face", dir + "haarcascade_frontalface_alt_tree.xml"));
        models.push_back(new Model("face", dir + "haarcascade_profileface.xml"));
        models.push_back(new Model("smile", dir + "haarcascade_smile.xml"));
        models.push_back(new Model("body", dir + "haarcascade_fullbody.xml"));
        models.push_back(new Model("body", dir + "haarcascade_upperbody.xml"));
        models.push_back(new Model("body", dir + "haarcascade_lowerbody.xml"));
    }

    ~Detector() {
        for (auto it = models.begin(); it != models.end(); it ++) {
            delete *it;
        }
    }

    void Detect(const cv::Mat &image, std::vector<Result> &out) {
        for (auto it = models.begin(); it != models.end(); it ++) {
            std::vector<cv::Rect> rects;
            (*it)->classifier.detectMultiScale(image, rects, scaleFactor, minNeighbors, flags, cv::Size(30, 30));
            for (auto r = rects.begin(); r != rects.end(); r ++) {
                out.push_back({*r, (*it)->type});
            }
        }
    }

private:
    static constexpr int flags = CV_HAAR_DO_CANNY_PRUNING | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
    static constexpr double scaleFactor = 1.3;
    static constexpr int minNeighbors = 3;

    std::vector<Model*> models;
};

class DetectApp : public zupi::Application {
public:
    DetectApp(int argc, char** argv)
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
        Detector detector(exeDir());
        cv::Mat image;

        for (int frame = 0; ; frame ++) {
            if (!m_vcap.read(image)) {
                break;
            }

            std::vector<Result> objs;
            detector.Detect(image, objs);
            if (!objs.empty()) {
                json j;
                j["size"] = { {"x", image.rows}, {"y", image.cols} };
                for (auto it = objs.begin(); it != objs.end(); it ++) {
                    j["objects"].push_back({
                        {"type", it->type},
                        {"range", { {"x", it->rect.x}, {"y", it->rect.y}, {"w", it->rect.width}, {"h", it->rect.height} } }
                    });
                }
                std::cout << j << std::endl;
            }
        }
        return 0;
    }

private:
    cv::VideoCapture m_vcap;
};

int main(int argc, char* argv[]) {
    DetectApp app(argc, argv);
    return app.main();
}
