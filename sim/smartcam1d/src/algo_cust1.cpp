#include <cmath>
#include <opencv2/core/mat.hpp>

#include "algo_cust1.h"

using namespace std;

class NeuralNet {
public:
    NeuralNet(int dimInput, int neurons)
    : m_wHid(dimInput, neurons, CV_64F),
      m_wOut(neurons, 1, CV_64F) {
          // TODO randomize weights
    }

    double output(const cv::Mat& input) const {
        cv::Mat m = input * m_wHid * m_wOut;
        auto v = m.at<double>(0, 0);
        return v / (1 + fabs(v));
    }

    void feedback(double out, double actual) {
        double err = out - actual;
        // deltaOut = err * out * (1 - out)
        // deltaHid = deltaOut * m_wOut
        // m_wOut -= alpha * deltaOut * (input * m_wHid)
        // m_wHid -= alpha * deltaHid * input


        /*
            Gn is delta of layer n
            En is error of layer n
            On is output of layer n
            In is input of layer n
            Tn is parameters of layer n

            Gn = En * sigmoid-derive(On)
            Tn -= a * (Gn * In)

            En = Gn+1 * Tn+1
        */
    }

private:
    cv::Mat m_wHid;
    cv::Mat m_wOut;
};

class QLearn {
public:
    struct Choice {
        int action;
        double q;
    };

    QLearn(int dimStates, int discreteActions, int neurons, double gamma = 0.9)
    : m_gamma(gamma) {
        for (int i = 0; i < discreteActions; i ++) {
            m_nets.push_back(new NeuralNet(dimStates, neurons));
        }
    }

    virtual ~QLearn() {
        for (auto net : m_nets) {
            delete net;
        }
    }

    Choice chooseMax(const cv::Mat& input) const {
        Choice c({-1, 0});
        for (int i = 0; i < m_nets.size(); i ++) {
            auto q = m_nets[i]->output(input);
            if (c.action < 0 || q > c.q) {
                c.action = i;
                c.q = q;
            }
        }
        return c;
    }

    void feedback(const Choice& c, const cv::Mat& input, double reward) {
        double q = reward + m_gamma * maxQval(input);
        m_nets[c.action]->feedback(c.q, q);
    }

private:
    int m_dimStates;
    int m_neurons;
    vector<NeuralNet*> m_nets;
    double m_gamma;

    double maxQval(const cv::Mat& input) const {
        return chooseMax(input).q;
    }
};

class Cust1Learner : public Learner, public TrainedModel {
public:
    Cust1Learner()
    : m_ql(2, 91, 6) {
    }

    virtual int learnAction(State s) {
        m_lastChoice = m_ql.chooseMax(cv::Mat(1, 1, CV_64F, &s.distance));
        return m_lastChoice.action;
    }

    virtual int action(State s) {
        return m_ql.chooseMax(cv::Mat(1, 1, CV_64F, &s.distance)).action;
    }

    virtual void feedback(State s, double reward) {
        m_ql.feedback(m_lastChoice, cv::Mat(1, 1, CV_64F, &s.distance), reward);
    }

    virtual TrainedModel* model() {
        return this;
    }

private:
    QLearn m_ql;
    QLearn::Choice m_lastChoice;
};

Cust1Algorithm::Cust1Algorithm(LearnModule *m)
: LearnAlgorithm("cust1", m) {

}

Learner* Cust1Algorithm::learner() {
    return new Cust1Learner();
}
