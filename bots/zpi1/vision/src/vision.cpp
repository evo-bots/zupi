#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "zupi/app.h"
#include "face_detector.h"

using namespace std;
using namespace cv;
using namespace zupi;
using namespace vision;

class Capture {
public:
    Capture(const string& source)
    : m_source(source), m_opened(false) {
    }

    bool opened() const { return m_opened; }

    virtual bool read(Mat& m) = 0;

protected:
    string m_source;
    bool m_opened;
};

class StreamCapture : public Capture {
public:
    StreamCapture(const string& source)
    : Capture(source) {
        m_opened = m_cap.open(source);
    }

    bool read(Mat& m) {
        return m_cap.read(m);
    }

private:
    VideoCapture m_cap;
};

class StillCapture : public Capture {
public:
    StillCapture(const string& source)
    : Capture(source) {
        m_opened = true;
    }

    bool read(Mat& m) {
        cv::VideoCapture vcap(m_source);
        if (vcap.isOpened()) {
            return vcap.read(m);
        }
        return false;
    }
};

class App : public zupi::Application {
public:
    App(int argc, char** argv)
        : zupi::Application(argc, argv) {
        options().add_options()
            ("show", "show recognized area", cxxopts::value<bool>())
            ("still", "use still instead of video", cxxopts::value<bool>())
            ("source", "media source", cxxopts::value<string>())
            ("args", "arguments for module", cxxopts::value<vector<string>>())
        ;
        options().parse_positional(vector<string>{"source", "args"});
    }

protected:
    virtual int run() {
        bool still = opt("still").as<bool>();
        bool show = opt("show").as<bool>();

        auto source = opt("source").as<string>();
        Capture *cap = still ? (Capture*)new StillCapture(source) : (Capture*)new StreamCapture(source);
        if (!cap->opened()) {
            cerr << "Error opening video stream or file" << endl;
            return 1;
        }

        FaceDetector detector(exeDir() + "/../share/OpenCV/", true, 2, 1, 60);
        cv::Mat image;

        if (show) {
            namedWindow("zpi1-vision");
        }
        while (true) {
            if (!cap->read(image)) {
                break;
            }
            DetectResult result(&detector, image);
            if (!result.empty()) {
                cout << result.json() << endl;
                cout.flush();
            }
            if (show) {
                for (auto& obj : result.objects) {
                    rectangle(image, obj.rc, obj.type == "smile" ? Scalar(0, 255, 0) : Scalar(255, 0, 0));
                }
                imshow("zpi1-vision", image);
                if (waitKey(500) >= 0) {
                    return 0;
                }
            }
        }
        if (show) {
            waitKey();
        }
        return 0;
    }
};

int main(int argc, char* argv[]) {
    return App(argc, argv).main();
}
