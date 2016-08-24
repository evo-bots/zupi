#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
#include <iostream>
#include <vector>

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
    static const int flags = CV_HAAR_DO_CANNY_PRUNING | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
    static const double scaleFactor = 1.3;
    static const int minNeighbors = 3;

    cv::CascadeClassifier fface, pface;
};

static int moveView(int pos, int delta, int limit) {
    int orig = pos;
    pos += delta;
    if (pos < 0) {
        pos = 0;
    }
    if (pos > limit) {
        pos = limit;
    }
    std::cout << "View: " << orig << " => " << pos << std::endl;
    return pos;
}

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

    //Create output window for displaying frames.
    //It's important to create this window outside of the `for` loop
    //Otherwise this window will be created automatically each time you call
    //`imshow(...)`, which is very inefficient.
    cv::namedWindow("Output Window");

    cv::Rect view;

    for (int frame = 0; ; frame ++) {
        if (!vcap.read(image)) {
            std::cout << "No frame" << std::endl;
            cv::waitKey();
            break;
        }

        if (view.width == 0 || view.height == 0) {
            view.width = 480;
            view.height = 360;
            view.x = (image.size().width - view.width) >> 1;
            view.y = (image.size().height - view.height) >> 1;
        }

        std::vector<cv::Rect> out, in;
        detector.Detect(image, out);
        detector.Detect(image(view), in);
        cv::rectangle(image, view, cv::Scalar(0, 0, 255));
        if (!out.empty()) {
            const cv::Rect &r = out[0];
            std::cout << frame << " Detected Out: " << r << std::endl;
            cv::rectangle(image, r, cv::Scalar(255, 0, 0));
        }
        if (!in.empty()) {
            const cv::Rect &r = in[0];
            std::cout << frame << " Detected In: " << r << std::endl;
            cv::rectangle(image, r + view.tl(), cv::Scalar(0, 255, 0));

            cv::Point m(view.width >> 1, view.height >> 1);
            cv::Point a(r.x + (r.width >> 1), r.y + (r.height >> 1));
            cv::Point d(a.x - m.x, a.y - m.y);
            if (d.x != 0) {
                view.x = moveView(view.x, d.x, image.size().width - view.width);
            }
            if (d.y != 0) {
                view.y = moveView(view.y, d.y, image.size().height - view.height);
            }
            cv::rectangle(image, view, cv::Scalar(0, 255, 255));
        }

        cv::imshow("Output Window", image);

        //if (!in.empty()) {
        //    cv::waitKey();
        //} else if (cv::waitKey(1) >= 0) {
        //    break;
        //}

        if (cv::waitKey(1) >= 0) {
            break;
        }
    }
}
