#pragma once

#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include "json.hpp"
#include "camera1d.h"
#include "app.h"

typedef ::sim::Pos1D State;

struct Advisor : ::zupi::Interface {
    virtual int action(State state) = 0;
};

struct TrainedModel : Advisor {
};

struct Learner : Advisor {
    virtual int learnAction(State state) = 0;
    virtual void feedback(State state, double reward) = 0;
    virtual TrainedModel* model() = 0;
};

class LearnAlgorithm;

class LearnModule : public Module {
public:
    LearnModule& addAlgorithm(const ::std::string&, LearnAlgorithm*);

    LearnAlgorithm* algorithm(const ::std::string& name) const;
    LearnAlgorithm* selectedAlgorithm() const;

protected:
    LearnModule(const ::std::string&, App*);

    virtual int run();
    virtual int run(LearnAlgorithm*) = 0;

private:
    ::std::map<::std::string, LearnAlgorithm*> m_algos;
};

class LearnAlgorithm {
public:
    virtual ~LearnAlgorithm() {}

    virtual Learner* learner() = 0;

protected:
    LearnAlgorithm(const ::std::string&, LearnModule*);
};

class SimulateModule : public LearnModule {
public:
    SimulateModule(App*);

    virtual int run(LearnAlgorithm*);

private:
    ::sim::Camera1D m_cam;
    ::sim::CaptureEnv1D m_cap;
    ::sim::Pos1D m_pos;
    ::std::mutex m_lock;
    ::std::condition_variable m_notifier;

    ::sim::Pos1D pos();
    double capture(State& s);
    void adjust(Learner*, State& s, bool training);
    void handleInput();
    void updateCam();
    void updatePos(double x, double y);
    void updateObj(const ::nlohmann::json&);
    void wait();
    void notify();
};

class TrainModule : public LearnModule {
public:
    TrainModule(App*);

    virtual int run(LearnAlgorithm*);
};

bool captureState(const ::sim::CaptureEnv1D&, const ::sim::Pos1D&, State&);
