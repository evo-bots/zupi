#include "timeline.h"

namespace sim {

TimeLine::TimeLine(int slice /* = MS */)
: m_slice(slice), m_time(0) {

}

TimeLine& TimeLine::reset(long t /* = 0 */) {
    m_time = t;
    m_delta = 0;
    m_actors.activateBy(*this);
    return *this;
}

TimeLine& TimeLine::step(int slices /* = 1 */) {
    if (slices != 0) {
        m_time += slices;
        m_delta = slices;
        m_actors.activateBy(*this);
    }
    return *this;
}

double TimeLine::distance(double velocity) const {
    // Vm(velocity per microsecond) = Vs(velocity per second)/1000000
    // d = delta * Vm * slice
    //   = delta * (Vs / (1000 * 1000)) * slice
    //   = delta * (Vs / 1000) * (slice / 1000)
    return m_delta * (velocity / 1000) * (m_slice / 1000);
}

}
