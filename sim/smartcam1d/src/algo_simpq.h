#pragma once

#include "simulate.h"

class SimpleQAlgorithm : public LearnAlgorithm {
public:
    SimpleQAlgorithm(LearnModule*);

    virtual Learner* learner();
};
