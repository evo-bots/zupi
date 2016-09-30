#include <cmath>
#include "zupi/common.h"
#include "algo_triangle.h"

using namespace std;

// this is actually not a learner

class TriangleLearner : public Learner, public TrainedModel {
public:
    TriangleLearner()
    : m_focal(50) {
    }

    virtual int learnAction(State s) {
        return action(s);
    }

    virtual int action(State s) {
        return (int)round(zupi::r2d(atan(s.distance / m_focal)));
    }

    virtual void feedback(State s, double reward) {
    }

    virtual TrainedModel* model() {
        return this;
    }

private:
    double m_focal;
};

TriangleAlgorithm::TriangleAlgorithm(LearnModule *m)
: LearnAlgorithm("triangle", m) {

}

Learner* TriangleAlgorithm::learner() {
    return new TriangleLearner();
}
