#pragma once

#include <string>
#include <map>
#include "zupi/app.h"

class Module;

class App : public ::zupi::Application {
public:
    App(int argc, char **argv);

    App& addModule(const ::std::string& name, Module* module);

protected:
    virtual int run();

private:
    ::std::map<::std::string, Module*> m_modules;
};

class Module {
public:
    virtual ~Module() {}

    const ::std::string& name() const { return m_name; }
    App& app() const { return *m_app; }

    ::cxxopts::Options& options() const { return app().options(); }
    const ::cxxopts::OptionDetails opt(const ::std::string& name) const { return app().opt(name); }

    virtual int run() = 0;

protected:
    Module(const ::std::string& name, App *app);

private:
    ::std::string m_name;
    App *m_app;
};
