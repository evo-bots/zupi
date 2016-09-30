#pragma once

#include "simulate.h"

class TriangleAlgorithm : public LearnAlgorithm {
public:
    TriangleAlgorithm(LearnModule*);

    virtual Learner* learner();
};
