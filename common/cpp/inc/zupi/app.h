#pragma once

#include <string>
#include "cxxopts.hpp"

namespace zupi {

class Application {
public:
    Application(int argc, char **argv);
    virtual int main();

    ::cxxopts::Options& options() { return m_options; }
    const ::cxxopts::OptionDetails opt(const ::std::string& name) const { return m_options[name]; }

    const ::std::string& name() const { return m_name; }
    const ::std::string& exeFile() const { return m_exeFile; }
    ::std::string exeDir() const;

protected:
    int m_argc;
    char** m_argv;

    virtual int run() { return 0; };

private:
    ::std::string m_name;
    ::std::string m_exeFile;
    ::cxxopts::Options m_options;
};

}
