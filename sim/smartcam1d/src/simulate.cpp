#include <memory>
#include <thread>
#include <mutex>
#include "json.hpp"
#include "events.h"
#include "simulate.h"
#include "algo_fido.h"

using namespace std;
using namespace zupi;
using namespace sim;
using json = nlohmann::json;

LearnModule::LearnModule(const ::std::string& name, App* app)
: Module(name, app) {
    new FidoAlgorithm(this);
}

LearnModule& LearnModule::addAlgorithm(const ::std::string& name, LearnAlgorithm* algo) {
    m_algos[name] = algo;
    return *this;
}

LearnAlgorithm* LearnModule::algorithm(const ::std::string& name) const {
    auto it = m_algos.find(name);
    return it == m_algos.end() ? NULL : it->second;
}

LearnAlgorithm* LearnModule::selectedAlgorithm() const {
    return algorithm(opt("algorithm").as<string>());
}

int LearnModule::run() {
    auto algo = selectedAlgorithm();
    if (algo) {
        return run(algo);
    }
    cerr << "invalid algorithm " << opt("algorithm").as<string>() << endl
        << "available algorithms are: " << endl;
    for (auto& item : m_algos) {
        cerr << "  " << item.first << endl;
    }
    return 1;
}

LearnAlgorithm::LearnAlgorithm(const ::std::string& name, LearnModule *module) {
    module->addAlgorithm(name, this);
}

bool captureState(const CaptureEnv1D &cap, const Pos1D& obj, State &p) {
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

SimulateModule::SimulateModule(App *app)
: LearnModule("sim", app) {
}

int SimulateModule::run(LearnAlgorithm *algo) {
    Camera1D cam;
    CaptureEnv1D cap(&cam);

    m_pos.distance = opt("distance").as<double>();
    m_pos.offset = opt("offset").as<double>();

    json actions;
    actions.push_back(ResetAction());
    actions.push_back(CornerObj("lt", -1000, 2000));
    actions.push_back(CornerObj("rt", 3000, 2000));
    actions.push_back(CornerObj("lb", -1000, -2000));
    actions.push_back(CornerObj("rb", 3000, -2000));
    actions.push_back(CameraObj("cam0").angle(r2d(cap.angle())));
    actions.push_back(DotObj("obj0", m_pos.x(), m_pos.y()));

    cout << actions << endl; cout.flush();

    unique_ptr<Learner> learner(algo->learner());

    thread input([this]() { handleInput(); });

    while (true) {
        State s;
        captureState(cap, pos(), s);
        cerr << "CAP angle: " << r2d(cap.angle())
            << ", distance: " << s.distance
            << ", offset: " << s.offset
            << endl;
        cerr.flush();
        if (s.distance >= 2) {
            adjustAndLearn(cap, learner.get(), s);
        } else {
            wait();
        }
    }

    return 0;
}

Pos1D SimulateModule::pos() {
    unique_lock<mutex> l(m_lock);
    return m_pos;
}

void SimulateModule::adjustAndLearn(CaptureEnv1D& cap, Learner *learner, State& s) {
    cap.angle(d2r(learner->action(s)));
    updateCam(cap);
    captureState(cap, pos(), s);
    auto reward = 1 - s.distance * 2 / cap.camera()->film();
    cerr << "RECAP angle: " << r2d(cap.angle())
        << ", offset: " << s.offset
        << ", reward: " << reward
        << endl;
    cerr.flush();
    learner->feedback(s, reward);
}

void SimulateModule::handleInput() {
    while (!cin.eof() && !cin.fail()) {
        string line;
        try {
            getline(cin, line);
            auto j = json::parse(line);
            if (!j.is_array()) {
                if (j.is_object()) {
                    j = {j};
                } else {
                    continue;
                }
            }

            double x, y;
            bool update = false;
            for (auto& item : j) {
                if (!item.is_object()) {
                    continue;
                }
                const string action = item["action"];
                if (action != "click") {
                    continue;
                }
                auto pos = item["position"];
                if (!pos.is_object()) {
                    continue;
                }
                if (!pos["x"].is_number() || !pos["y"].is_number()) {
                    continue;
                }

                x = pos["x"];
                y = pos["y"];
                update = true;
            }

            if (update) {
                updatePos(x, y);
                notify();
            }
        } catch (const invalid_argument& e) {
            // ignored
            continue;
        }
    }
}

void SimulateModule::updateCam(CaptureEnv1D& cap) {
    json j;
    j.push_back(CameraObj("cam0").angle(r2d(cap.angle())));
    unique_lock<mutex> l(m_lock);
    cout << j << endl; cout.flush();
}

void SimulateModule::updatePos(double x, double y) {
    unique_lock<mutex> l(m_lock);
    m_pos.distance = x;
    m_pos.offset = y;
    json j;
    j.push_back(DotObj("obj0", m_pos.x(), m_pos.y()));
    cout << j << endl; cout.flush();
}

void SimulateModule::wait() {
    unique_lock<mutex> l(m_lock);
    m_notifier.wait(l);
}

void SimulateModule::notify() {
    m_notifier.notify_all();
}
