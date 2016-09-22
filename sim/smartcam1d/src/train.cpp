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
    Pos1D p;

    unique_ptr<Learner> learner(algo->learner());

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

            cap.angle(d2r(learner->action(p)));
            //auto prevDist = p.distance;
            captureState(cap, objs[i], p);
            //auto reward = (prevDist - p.distance) * 2 / cap.camera()->film();
            auto reward = 1 - p.distance * 2 / cap.camera()->film();
            cout << "    " << reward << " new angle: " << r2d(cap.angle())
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
            learner->feedback(p, reward);
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
