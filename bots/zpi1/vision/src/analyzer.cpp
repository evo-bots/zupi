#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

class Detector {
public:
    Detector()
        : fface("/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml"),
          pface("/usr/share/opencv/haarcascades/haarcascade_profileface.xml") {
    }

    void Detect(const cv::Mat &image, std::vector<cv::Rect> &out) {
        fface.detectMultiScale(image, out, scaleFactor, minNeighbors, flags, cv::Size(60, 60));
        if (out.empty()) {
            pface.detectMultiScale(image, out, scaleFactor, minNeighbors, flags, cv::Size(60, 60));
        }
    }

private:
    static constexpr int flags = CV_HAAR_DO_CANNY_PRUNING | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
    static constexpr double scaleFactor = 1.3;
    static constexpr int minNeighbors = 3;

    cv::CascadeClassifier fface, pface;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Missing video source" << std::endl;
        return 1;
    }

    cv::VideoCapture vcap;
    cv::Mat image;

    //open the video stream and make sure it's opened
    if (!vcap.open(argv[1])) {
        std::cerr << "Error opening video stream or file" << std::endl;
        return 1;
    }


    Detector detector;
    for (int frame = 0; ; frame ++) {
        if (!vcap.read(image)) {
            break;
        }

        std::vector<cv::Rect> objs;
        detector.Detect(image, objs);
        if (!objs.empty()) {
            json j;
            j["size"] = { {"x", image.rows}, {"y", image.cols} };
            for (std::vector<cv::Rect>::iterator it = objs.begin(); it != objs.end(); it ++) {
                j["objects"].push_back({ {"x", it->x}, {"y", it->y}, {"w", it->width}, {"h", it->height} });
            }
            std::cout << j << std::endl;
        }
    }
}
