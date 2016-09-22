#pragma once

#include "app.h"

class CaptureModule : public Module {
public:
    CaptureModule(App*);

    virtual int run();
};
