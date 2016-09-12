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
            ("m,mode", "Run mode", cxxopts::value<string>()->default_value("cap"))
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
    Pos1D obj(2000, 100), p;
    rl::FidoControlSystem learner(1, {-90}, {90}, 180);

    captureState(cap, obj, p);
    for (int i = 0; i < 30; i ++) {
        cout << i << ". state: " << p.distance << ", offset: " << p.offset << endl;
        rl::Action action = learner.chooseBoltzmanAction({p.distance}, 0.2);
        cap.angle(d2r(action[0]));
        //auto prevDist = p.distance;
        captureState(cap, obj, p);
        //auto reward = (prevDist - p.distance) * 2 / cap.camera()->film();
        auto reward = (1 - p.distance * 2 / cap.camera()->film()) * 2 - 1;
        cout << "    new angle: " << r2d(cap.angle()) << ", reward: " << reward << endl;
        learner.applyReinforcementToLastAction(reward, {p.distance});
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
        rl::Action action = learner.chooseBoltzmanAction({p.offset}, 0.05);
        cap.angle(action[0] * M_PI / 2);

        json j;
        j.push_back(CameraObj("cam0").angle(r2d(cap.angle())));
        cout << j << endl; cout.flush();

        //auto prevDist = p.distance;
        //auto inFilm = captureState(cap, obj, p);
        //auto reward = inFilm ? (prevDist - p.distance) * 2 / cap.camera()->film() : -1;
        captureState(cap, obj, p);
        auto reward = (1 - p.distance * 2 / cap.camera()->film()) * 2 - 1;
        cerr << "    new angle: " << r2d(cap.angle()) << ", offset: " << p.offset << ", reward: " << reward << endl;

        learner.applyReinforcementToLastAction(reward, {p.offset});
    }

    return 0;
}

int main(int argc, char **argv) {
    App app(argc, argv);
    return app.main();
}
