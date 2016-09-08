#ifndef _APP_H
#define _APP_H

#include <string>

class Application {
public:
    Application(int argc, char **argv);
    virtual int run();

    ::std::string exeFile() const { return m_exeFile; }
    ::std::string exeDir() const;

protected:
    int m_argc;
    char** m_argv;

private:
    ::std::string m_exeFile;
};

#endif
