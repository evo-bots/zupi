#ifndef _COMMON_H
#define _COMMON_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <list>
#include <algorithm>
#include <functional>

namespace zupi {

// degrees to radians
constexpr double d2r(double degrees) {
    return degrees * M_PI / 180;
}

// radians to degrees
constexpr double r2d(double radians) {
    return radians * 180 / M_PI;
}

// Base Interface type
struct Interface {
    virtual ~Interface() { }
};

template<typename T>
class Reactors {
public:
    Reactors& add(T reactor) { m_reactors.push_back(reactor); return *this; }
    void activate(::std::function<void (T)> activator) const {
        ::std::for_each(m_reactors.begin(), m_reactors.end(), activator);
    }

private:
    ::std::list<T> m_reactors;
};

template<typename T>
class Observers : public Reactors<::std::function<void (T)>> {
public:
    void activateBy(T owner) {
        this->activate([=](auto a) { a(owner); });
    }
};

}

#endif
