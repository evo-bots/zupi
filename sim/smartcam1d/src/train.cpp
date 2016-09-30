#include <iostream>
#include "Fido/FidoControlSystem.h"
#include "camera1d.h"
#include "simulate.h"

using namespace std;
using namespace zupi;
using namespace sim;

TrainModule::TrainModule(App *app)
: LearnModule("train", app) {
}

int TrainModule::run(LearnAlgorithm *algo) {
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

    unique_ptr<Learner> learner(algo->learner());
    State s;
    auto training = true;
    int tries = 0;
    int samples = sizeof(objs)/sizeof(objs[0]);
    captureState(cap, objs[0], s);
    for (int i = 0; i < samples; i ++) {
        for (int j = 0; true; j ++) {
            cout << i << "." << j << " state: " << s.distance
                << ", offset: " << s.offset
                << ", training: " << training
                << endl;
            //rl::Action action = training ?
            //    learner.chooseBoltzmanActionDynamic({p.distance}) :
            //    learner.chooseBoltzmanAction({p.distance}, 0.01);

            captureUpdateAngle(cap, s, training ? learner->learnAction(s) : learner->action(s));

            //auto prevDist = p.distance;
            captureState(cap, objs[i], s);
            //auto reward = (prevDist - p.distance) * 2 / cap.camera()->film();
            auto reward = 1 - s.distance * 2 / cap.camera()->film();
            cout << "    " << reward << " new angle: " << s.angle
                << ", state: " << s.distance
                << ", offset: " << s.offset
                << endl;
            if (training) {
                learner->feedback(s, reward);
            }
            if (reward >= 0.9) {
                training = false;
                cout << "GOAL ACHIEVED!!! " << j << " tries" << endl;
                tries += j;
                break;
            } else if (!training) {
                training = true;
                cout << "TRAINING" << endl;
            }
        }
    }

    cout << "AvgTries " << double(tries) / samples << endl;

    cout << "Validation ..." << endl;
    cap.angle(0);
    for (int i = 0; i < samples; i ++) {
        double a0 = r2d(cap.angle());
        captureState(cap, objs[i], s);
        double off0 = s.offset;
        cap.angle(d2r(learner->action(s)));
        captureState(cap, objs[i], s);
        cout << i << ". " << off0 << " "
            << a0 << " -> " << s.angle
            << ": " << s.offset
            << endl;
    }

    return 0;
}
