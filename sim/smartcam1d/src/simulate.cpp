#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include "events.h"
#include "simulate.h"
#include "algo_fido.h"
#include "algo_cust1.h"
#include "algo_simpq.h"
#include "algo_triangle.h"

using namespace std;
using namespace zupi;
using namespace sim;
using json = nlohmann::json;

LearnModule::LearnModule(const ::std::string& name, App* app)
: Module(name, app) {
    new FidoAlgorithm(this);
    new Cust1Algorithm(this);
    new SimpleQAlgorithm(this);
    new TriangleAlgorithm(this);
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
    p.angle = r2d(cap.angle());
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

void captureUpdateAngle(CaptureEnv1D &cap, const State &s, double delta) {
    double a = r2d(cap.angle());
    if (s.offset < 0) {
        a += delta;
    } else {
        a -= delta;
    }
    if (a > 90) {
        a = 90;
    }
    if (a < -90) {
        a = -90;
    }
    cap.angle(d2r(a));
}

SimulateModule::SimulateModule(App *app)
: LearnModule("sim", app), m_cap(&m_cam) {
    options().add_options()
        ("t,threshold", "good threshold", cxxopts::value<double>()->default_value("2"))
    ;
}

int SimulateModule::run(LearnAlgorithm *algo) {
    m_pos.distance = opt("distance").as<double>();
    m_pos.offset = opt("offset").as<double>();
    double threshold = opt("threshold").as<double>();

    json actions;
    actions.push_back(ResetAction());
    actions.push_back(CornerObj("lt", -1000, 2000));
    actions.push_back(CornerObj("rt", 3000, 2000));
    actions.push_back(CornerObj("lb", -1000, -2000));
    actions.push_back(CornerObj("rb", 3000, -2000));
    actions.push_back(CameraObj("cam0").angle(r2d(m_cap.angle())));
    actions.push_back(DotObj("obj0", m_pos.x(), m_pos.y()));

    cout << actions << endl; cout.flush();

    unique_ptr<Learner> learner(algo->learner());

    thread input([this]() { handleInput(); });

    bool training = true;
    while (true) {
        State s;
        capture(s);
        cerr << "CAP angle: " << s.angle
            << ", distance: " << s.distance
            << ", offset: " << s.offset
            << endl;
        cerr.flush();
        bool good = s.distance < threshold;
        if (!good) {
            adjust(learner.get(), s, training);
            if (s.distance >= threshold) {
                training = true;
            }
        } else {
            training = false;
            wait();
        }
    }

    return 0;
}

Pos1D SimulateModule::pos() {
    unique_lock<mutex> l(m_lock);
    return m_pos;
}

double SimulateModule::capture(State &s) {
    captureState(m_cap, pos(), s);
    auto reward = 1 - s.distance * 2 / m_cap.camera()->film();
    ostringstream str;
    str << "angle: " << s.angle
        << ", offset: " << s.offset;
    json j;
    j.push_back(RewardObj("reward0", reward).rect(-900, 1900, 160, 80));
    j.push_back(LabelObj("state0", str.str()).rect(-700, 1900, 800, 80));
    updateObj(j);
    return reward;
}

void SimulateModule::adjust(Learner *learner, State& s, bool training) {
    captureUpdateAngle(m_cap, s, training ? learner->learnAction(s) : learner->action(s));
    // simulate delay of cap adjust
    this_thread::sleep_for(chrono::milliseconds(100));
    updateCam();
    auto reward = capture(s);
    cerr << "RECAP angle: " << s.angle
        << ", offset: " << s.offset
        << ", reward: " << reward
        << endl;
    cerr.flush();
    if (training) {
        LabelObj obj("training", "Training");
        updateObj(json({obj.rect(2700, 1900, 250, 80).style("training")}));
        learner->feedback(s, reward);
        updateObj(json({obj.content("Trained").style("trained")}));
    }
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

void SimulateModule::updateCam() {
    json j;
    j.push_back(CameraObj("cam0").angle(r2d(m_cap.angle())));
    updateObj(j);
}

void SimulateModule::updatePos(double x, double y) {
    unique_lock<mutex> l(m_lock);
    m_pos.distance = x;
    m_pos.offset = y;
    json j;
    j.push_back(DotObj("obj0", m_pos.x(), m_pos.y()));
    cout << j << endl; cout.flush();
}

void SimulateModule::updateObj(const json& j) {
    cout << j << endl; cout.flush();
}

void SimulateModule::wait() {
    unique_lock<mutex> l(m_lock);
    m_notifier.wait(l);
}

void SimulateModule::notify() {
    m_notifier.notify_all();
}
