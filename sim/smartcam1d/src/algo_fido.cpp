#include <cmath>
#include "Fido/FidoControlSystem.h"
#include "algo_fido.h"

using namespace std;

class FidoLearner : public Learner, public TrainedModel {
public:
    FidoLearner()
    : m_learner(1, {0}, {1}, 90) {
    }

    virtual int action(State s) {
        auto a = m_learner.chooseBoltzmanActionDynamic({s.distance});
        return (int)round((s.offset < 0 ? 90 : -90) * a[0]);
    }

    virtual void feedback(State s, double reward) {
        m_learner.applyReinforcementToLastAction(reward, {s.distance});
    }

    virtual TrainedModel *model() {
        return this;
    }

private:
    rl::FidoControlSystem m_learner;
};

FidoAlgorithm::FidoAlgorithm(LearnModule *m)
: LearnAlgorithm("fido", m) {
}

Learner* FidoAlgorithm::learner() {
    return new FidoLearner();
}
