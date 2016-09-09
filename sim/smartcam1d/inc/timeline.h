#ifndef _SIM_TIMELINE_H
#define _SIM_TIMELINE_H

#include "common.h"

namespace sim {

class TimeLine {
public:
    typedef ::std::function<void (const TimeLine&)> Actor;

    static constexpr int MS = 1000;

    TimeLine(int slice = MS);

    TimeLine& reset(long t = 0);
    TimeLine& step(int slices = 1);

    int slice() const { return m_slice; }
    TimeLine& slice(int s) { m_slice = s; return *this; }

    long current() const { return m_time; }
    long delta() const { return m_delta; }

    // calculate distance moved during time change
    // the unit of velocity is per second
    double distance(double velocity) const;

    TimeLine& act(Actor a) { m_actors.add(a); return *this; }

private:
    int m_slice;   // minimum time slice in microsecond, 1e-6s
    long m_time;   // current time
    long m_delta;  // previous delta
    ::zupi::Observers<const TimeLine&> m_actors;
};

}

#endif
