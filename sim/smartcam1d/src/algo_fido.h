#pragma once

#include "simulate.h"

class FidoAlgorithm : public LearnAlgorithm {
public:
    FidoAlgorithm(LearnModule*);

    virtual Learner* learner();
};
