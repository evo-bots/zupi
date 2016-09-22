#include <iostream>
#include <algorithm>
#include "app.h"
#include "capture.h"
#include "simulate.h"

using namespace std;
using namespace zupi;

Module::Module(const string& name, App *app)
: m_name(name), m_app(app) {
    app->addModule(name, this);
}

App::App(int argc, char **argv)
: Application(argc, argv) {
    options().add_options()
        ("d,distance", "object distance in mm", cxxopts::value<double>()->default_value("2000"))
        ("o,offset", "object offset in mm", cxxopts::value<double>()->default_value("50"))
        ("a,angle", "camera angle in degrees to rotate", cxxopts::value<double>()->default_value("0"))
        ("l,algorithm", "learning algorithm", cxxopts::value<string>())
        ("module", "run module", cxxopts::value<string>())
        ("args", "arguments for module", cxxopts::value<vector<string>>())
    ;
    options().parse_positional(vector<string>{"module", "args"});

    new CaptureModule(this);
    new TrainModule(this);
    new SimulateModule(this);
}

App& App::addModule(const ::std::string &name, Module *module) {
    m_modules[name] = module;
    return *this;
}

int App::run() {
    srand(time(NULL));
    auto name = opt("module").as<string>();
    auto module = m_modules.find(name);
    if (module != m_modules.end()) {
        return module->second->run();
    }
    cerr << "invalid module " << name << endl
        << "available modules are: " << endl;
    for (auto& item : m_modules) {
        cerr << "  " << item.first << endl;
    }
    return 1;
}

int main(int argc, char **argv) {
    return App(argc, argv).main();
}
