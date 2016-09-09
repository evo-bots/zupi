#ifndef _SIM_SERVO_H
#define _SIM_SERVO_H

#include "timeline.h"

namespace sim {

class Servo {
public:
    typedef ::std::function<void (const Servo&)> Actor;

    static constexpr double SPD600 = ::zupi::d2r(600);

    Servo(double speed = SPD600, double initialAngle = 0);

    Servo& reset();

    double speed() const { return m_speed; }
    Servo& speed(double s) { m_speed = s; return *this; }

    double angle() const { return m_angle; }
    double moveTo() const { return m_target; }
    Servo& moveTo(double t) { m_target = t; return *this; }
    Servo& moveBy(double dt) { m_target = m_angle + dt; return *this; }

    bool moving() const { return m_angle != m_target; }

    Servo& act(Actor a) { m_actors.add(a); return *this; }

    TimeLine::Actor actor() const {
        return [this](const TimeLine& t) { updateTime(t); };
    }

protected:
    void updateTime(const TimeLine&);

private:
    double m_speed;     // speed in radians per second
    double m_initialAngle;
                        // initial angle for reset
    double m_angle;     // current angle, 0 is the original angle
    double m_target;    // the target angle

    ::zupi::Observers<const Servo&> m_actors;
};

}

#endif
