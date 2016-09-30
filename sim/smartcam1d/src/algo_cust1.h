#pragma once

#include "simulate.h"

class Cust1Algorithm : public LearnAlgorithm {
public:
    Cust1Algorithm(LearnModule*);

    virtual Learner* learner();
};
