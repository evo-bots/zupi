#pragma once

#include <map>
#include <string>
#include <vector>
#include "zupi/app.h"
#include "vision.h"

class Detection {
public:
    Detection();

    void init(::zupi::Application*);
    ::vision::Detector* get(const ::std::string& name) const;
    ::std::vector<::std::string> names() const;

private:
    ::std::map<::std::string, ::vision::Detector*> m_detectors;
};

class DetectApp : public ::zupi::Application {
public:
    DetectApp(int argc, char **argv)
    : ::zupi::Application(argc, argv) {
        m_detection.init(this);
    }

    ::std::vector<::std::string> detectors() const {
        return m_detection.names();
    }

    ::vision::Detector* detector(const ::std::string& name) const {
        return m_detection.get(name);
    }

private:
    Detection m_detection;
};
