#include <iostream>
#include <chrono>
#include <map>
#include "detection.h"

using namespace std;
using namespace cv;
using namespace zupi;
using namespace vision;

class App : public DetectApp {
public:
    App(int argc, char** argv)
        : DetectApp(argc, argv) {
        options().add_options()
            ("detector", "run detector", cxxopts::value<string>())
            ("source", "media source", cxxopts::value<string>())
            ("args", "arguments for module", cxxopts::value<vector<string>>())
        ;
        options().parse_positional(vector<string>{"detector", "source", "args"});
    }

protected:
    virtual int run() {
        auto d = detector(opt("detector").as<string>());
        if (!d) {
            cerr << "invalid detector, choose one of" << endl;
            for (auto& name : detectors()) {
                cerr << "    " << name << endl;
            }
            return 1;
        }

        auto source = opt("source").as<string>();
        Mat image = imread(source);
        if (image.empty()) {
            cerr << "invalid source" << endl;
            return 1;
        }

        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < 50; i ++) {
            DetectResult(d, image);
        }
        auto elapsed = chrono::high_resolution_clock::now() - start;
        long long microseconds = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        cout << microseconds << " " << microseconds / 50 << endl;

        cout << DetectResult(d, image).json() << endl;
        return 0;
    }
};

int main(int argc, char* argv[]) {
    return App(argc, argv).main();
}
