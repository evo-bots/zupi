#include <map>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <iostream>
#include "algo_simpq.h"

using namespace std;

static int mapState(const State &s) {
    return (int)round(s.distance * 10);
}

static int mapAction(const State &s, int action) {
    return s.offset < 0 ? action : - action;
}

class SimpleQLearner : public Learner, public TrainedModel {
public:
    SimpleQLearner()
    : m_actions(91), m_gamma(0.9) {
    }

    virtual int learnAction(State s) {
        m_lastAction = rand() * m_actions / RAND_MAX;
        m_lastState = mapState(s);
        return mapAction(s, m_lastAction);
    }

    virtual int action(State s) {
        auto it = m_q.find(mapState(s));
        if (it == m_q.end()) {
            return learnAction(s);
        }
        int a = max_element(it->second.begin(), it->second.end()) - it->second.begin();
        return mapAction(s, a);
    }

    virtual void feedback(State s, double reward) {
        auto it = m_q.find(m_lastState);
        if (it == m_q.end()) {
            vector<double> qvals(m_actions, 0);
            it = m_q.insert(pair<double, vector<double>>(m_lastState, move(qvals))).first;
        }
        vector<double> &qvals = it->second;
        qvals[m_lastAction] = reward + m_gamma * maxQval(mapState(s));

        /*
        for (auto& p : m_q) {
            cout << p.first << endl;
            for (auto i = 0; i < p.second.size(); i ++) {
                if (i % 10 == 0) {
                    cout << "   ";
                }
                cout << " " << p.second[i];
                if ((i + 1) % 10 == 0) {
                    cout << endl;
                }
            }
            cout << endl;
        }
        */
    }

    virtual TrainedModel* model() {
        return this;
    }

private:
    size_t m_actions;
    double m_gamma;
    map<int, vector<double>> m_q;
    int m_lastAction;
    int m_lastState;

    double maxQval(int state) const {
        auto it = m_q.find(state);
        if (it == m_q.end()) {
            return 0;
        }
        return *max_element(it->second.begin(), it->second.end());
    }
};

SimpleQAlgorithm::SimpleQAlgorithm(LearnModule *m)
: LearnAlgorithm("simpq", m) {

}

Learner* SimpleQAlgorithm::learner() {
    return new SimpleQLearner();
}
