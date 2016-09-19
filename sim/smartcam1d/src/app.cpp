#include <chrono>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "Fido/FidoControlSystem.h"
#include "app.h"
#include "camera1d.h"
#include "servo.h"
#include "events.h"

using namespace std;
using namespace zupi;
using namespace sim;

class App : public Application {
public:
    App(int argc, char **argv)
        : Application(argc, argv) {
        options().add_options()
            ("m,mode", "Run mode", cxxopts::value<string>()->default_value("sim"))
            ("d,distance", "object distance in mm", cxxopts::value<int>()->default_value("2000"))
            ("o,offset", "object offset in mm", cxxopts::value<int>()->default_value("50"))
            ("a,angle", "camera angle in degrees to rotate", cxxopts::value<double>()->default_value("0"))
        ;
    }

protected:
    virtual int run();

private:
    int runSimulator();
    int runTrainer();
    int runCapture();
};

int App::run() {
    srand(time(NULL));
    auto mode = options()["mode"].as<string>();
    if (mode == "cap") {
        return runCapture();
    }
    if (mode == "train") {
        return runTrainer();
    }
    return runSimulator();
}

int App::runCapture() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);

    Pos1D obj(options()["distance"].as<int>(), options()["offset"].as<int>());
    auto p = cap.angle(d2r(options()["angle"].as<double>())).capture(obj);
    json j = {{"distance", p.distance}, {"offset", p.offset}, {"in", cam.onFilm(p)}};
    cout << j << endl;
}

bool captureState(const CaptureEnv1D &cap, const Pos1D& obj, Pos1D &p) {
    p = cap.capture(obj);
    if (!cap.camera()->onFilm(p)) {
        p.distance = cap.camera()->film()/2;
        if (p.offset < 0) {
            p.offset = -p.distance;
        } else {
            p.offset = p.distance;
        }
        return false;
    } else {
        p.distance = std::abs(p.offset);
        return true;
    }
}

int App::runTrainer() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);
    Pos1D objs[] = {
        Pos1D(2000, 100),
        Pos1D(2000, -100),
        Pos1D(2000, 200),
        Pos1D(2000, -200),
        Pos1D(2000, 50),
        Pos1D(2000, 150),
        Pos1D(2000, -150),
        Pos1D(2000, 220),
        Pos1D(2000, -220),
        Pos1D(2000, -80),
    };
    Pos1D p;
    rl::FidoControlSystem learner(1, {0}, {1}, 90);

    auto training = true;
    captureState(cap, objs[0], p);
    for (int i = 0; i < sizeof(objs)/sizeof(objs[0]); i ++) {
        for (int j = 0; true; j ++) {
            cout << i << "." << j << " state: " << p.distance
                << ", offset: " << p.offset
                << ", training: " << training
                << endl;
            //rl::Action action = training ?
            //    learner.chooseBoltzmanActionDynamic({p.distance}) :
            //    learner.chooseBoltzmanAction({p.distance}, 0.01);
            rl::Action action = learner.chooseBoltzmanActionDynamic({p.distance});
            auto d = (p.offset < 0 ? 90 : -90) * action[0];
            cap.angle(d2r(d));
            //auto prevDist = p.distance;
            captureState(cap, objs[i], p);
            //auto reward = (prevDist - p.distance) * 2 / cap.camera()->film();
            auto reward = 1 - p.distance * 2 / cap.camera()->film();
            cout << "    " << reward << " new angle: " << r2d(cap.angle())
                << ", action: " << action[0]
                << ", state: " << p.distance
                << ", offset: " << p.offset
                << endl;
            if (reward >= 0.9) {
                training = false;
                cout << "GOAL ACHIEVED!!!" << endl;
                break;
            } else if (!training) {
                training = true;
                cout << "TRAINING" << endl;
            }
            learner.applyReinforcementToLastAction(reward, {p.distance});
        }

/*
        cout << "Validation ..." << endl;
        for (int j = 0; j < sizeof(objs)/sizeof(objs[0]); j ++) {
            rl::Action action = learner.chooseBoltzmanAction({p.distance}, 0.01);
            auto d = (p.offset < 0 ? 90 : -90) * action[0];
            cap.angle(d2r(d));
            captureState(cap, objs[j], p);
            auto reward = 1 - p.distance * 2 / cap.camera()->film();
            cout << j << " " << reward << " angle: " << r2d(cap.angle())
                << ", action: " << action[0]
                << ", state: " << p.distance
                << ", offset: " << p.offset
                << endl;
        }
*/
    }

    return 0;
}

int App::runSimulator() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);
    Pos1D obj(2000, 100), p;

    json actions;
    actions.push_back(CornerObj("lt", -1000, 2000));
    actions.push_back(CornerObj("rt", 3000, 2000));
    actions.push_back(CornerObj("lb", -1000, -2000));
    actions.push_back(CornerObj("rb", 3000, -2000));
    actions.push_back(CameraObj("cam0").angle(r2d(cap.angle())));
    actions.push_back(DotObj("obj0", obj.x(), obj.y()));

    cout << actions << endl; cout.flush();

    rl::FidoControlSystem learner(1, {-1}, {1}, 180);

    captureState(cap, obj, p);
    for (int i = 0; i < 1000; i ++) {
        if (p.distance < 2.3) {
            obj.offset = ((rand() % 300) - 150) * 10;
            cerr << "    re-position: " << obj.offset << endl;

            json j;
            j.push_back(DotObj("obj0", obj.x(), obj.y()));
            cout << j << endl; cout.flush();
        }

        cerr << i << ". state: " << p.distance << ", offset: " << p.offset << endl;
        rl::Action action = learner.chooseBoltzmanActionDynamic({p.offset});
        cap.angle(action[0] * M_PI / 2);

        json j;
        j.push_back(CameraObj("cam0").angle(r2d(cap.angle())));
        cout << j << endl; cout.flush();

        //auto prevDist = p.distance;
        //auto inFilm = captureState(cap, obj, p);
        //auto reward = inFilm ? (prevDist - p.distance) * 2 / cap.camera()->film() : -1;
        captureState(cap, obj, p);
        auto reward = 1 - p.distance * 2 / cap.camera()->film();
        cerr << "    new angle: " << r2d(cap.angle()) << ", offset: " << p.offset << ", reward: " << reward << endl;

        learner.applyReinforcementToLastAction(reward, {p.offset});
    }

    return 0;
}

int main(int argc, char **argv) {
    App app(argc, argv);
    return app.main();
}
