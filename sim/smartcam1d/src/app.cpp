#include <vector>
#include <iostream>
#include "app.h"
#include "camera1d.h"
#include "servo.h"
#include "Fido/FidoControlSystem.h"

using namespace std;
using namespace zupi;
using namespace sim;

class App : public Application {
public:
    App(int argc, char **argv)
        : Application(argc, argv) {
        options().add_options()
            ("m,mode", "Run mode", cxxopts::value<string>()->default_value("cap"))
        ;
    }

protected:
    virtual int run();

private:
    int runTrainer();
    int runCapture();
};

int App::run() {
    auto mode = options()["mode"].as<string>();
    if (mode == "cap") {
        return runCapture();
    }
    return runTrainer();
}

int App::runCapture() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);

    // Let's generate a few positions
    vector<Pos1D> objs;
    objs.push_back(Pos1D(2000, 50));
    objs.push_back(Pos1D(1500, 50));
    objs.push_back(Pos1D(1500, 300));

    for (auto it = objs.begin(); it != objs.end(); it ++) {
        cout << "capturing " << it->distance << ", " << it->offset << ": ";
        auto capPos = cap.capture(*it);
        if (capPos.distance < 0) {
            cout << "failed";
        } else {
            cout << capPos.offset;
        }
        cout << endl;
    }

    auto p = cap.angle(d2r(30)).capture(objs[0]);
    cout << "rotate ccw 30: " << p.offset << ", " << p.distance << endl;
    p = cap.angle(d2r(-30)).capture(objs[0]);
    cout << "rotate  cw 30: " << p.offset << ", " << p.distance << endl;
}

Pos1D captureState(const CaptureEnv1D &cap, const Pos1D& obj) {
    auto p = cap.capture(obj);
    if (!cap.camera()->onFilm(p)) {
        p.distance = 0;
    } else {
        p.distance = cap.camera()->film()/2 - std::abs(p.offset);
    }
    return p;
}

int App::runTrainer() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);
    Pos1D obj(2000, 100);
    rl::FidoControlSystem learner(1, {-90}, {90}, 180);

    auto p = captureState(cap, obj);
    for (int i = 0; i < 50; i ++) {
        cout << i << ". state: " << p.distance << ", offset: " << p.offset << endl;
        rl::Action action = learner.chooseBoltzmanAction({p.offset}, 0.2);
        cap.angle(d2r(action[0]));
        auto prevDist = p.distance;
        p = captureState(cap, obj);
        auto reward = p.distance - prevDist;
        cout << "    new angle: " << r2d(cap.angle()) << ", reward: " << reward << endl;
        learner.applyReinforcementToLastAction(reward, {p.offset});
    }

    return 0;
}

int main(int argc, char **argv) {
    App app(argc, argv);
    return app.main();
}
