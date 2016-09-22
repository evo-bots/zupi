#include <iostream>
#include "json.hpp"
#include "camera1d.h"
#include "capture.h"

using namespace std;
using namespace zupi;
using namespace sim;
using json = nlohmann::json;

CaptureModule::CaptureModule(App *app)
: Module("cap", app) {

}

int CaptureModule::run() {
    Camera1D cam;
    CaptureEnv1D cap(&cam);

    Pos1D obj(opt("distance").as<double>(), opt("offset").as<double>());

    auto p = cap.angle(d2r(opt("angle").as<double>())).capture(obj);
    json j = {{"distance", p.distance}, {"offset", p.offset}, {"in", cam.onFilm(p)}};
    cout << j << endl;

    return 0;
}
