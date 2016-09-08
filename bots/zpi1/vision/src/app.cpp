#include <unistd.h>
#include <limits.h>
#include "app.h"

Application::Application(int argc, char **argv)
: m_argc(argc), m_argv(argv) {
    char exePath[PATH_MAX];
    if (readlink("/proc/self/exe", exePath, sizeof(exePath)) >= 0) {
        exePath[sizeof(exePath) - 1] = 0;
        m_exeFile = exePath;
    } else {
        m_exeFile = argv[0];
    }
}

int Application::run() {
    return 0;
}

::std::string Application::exeDir() const {
    auto pos = m_exeFile.find_last_of("/");
    if (pos != ::std::string::npos) {
        return m_exeFile.substr(0, pos);
    }
    return "";
}
