#include <unistd.h>
#include <limits.h>
#include <exception>
#include <iostream>
#include "app.h"

namespace zupi {

static const char* appName(const char *argv0) {
    const char *p = strrchr(argv0, '/');
    return p == NULL ? argv0 : p;
}

static ::std::string exeFullPath(const char *argv0) {
    char exePath[PATH_MAX];
    if (readlink("/proc/self/exe", exePath, sizeof(exePath)) >= 0) {
        exePath[sizeof(exePath) - 1] = 0;
        return exePath;
    } else {
        return argv0;
    }
}

Application::Application(int argc, char **argv)
: m_argc(argc), m_argv(argv),
  m_name(appName(argv[0])), m_exeFile(exeFullPath(argv[0])),
  m_options(m_name) {
}

int Application::main() {
    m_options.parse(m_argc, m_argv);
    try {
        return run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}

::std::string Application::exeDir() const {
    auto pos = m_exeFile.find_last_of("/");
    if (pos != ::std::string::npos) {
        return m_exeFile.substr(0, pos);
    }
    return "";
}

}
