#include "servo.h"

namespace sim {

Servo::Servo(double speed /* = SPD600 */, double initialAngle /* = 0 */)
: m_speed(speed), m_initialAngle(initialAngle) {
    m_target = m_angle = m_initialAngle;
}

Servo& Servo::reset() {
    m_angle = m_target = m_initialAngle;
    m_actors.activateBy(*this);
}

void Servo::updateTime(const TimeLine& t) {
    auto delta = t.delta();
    if (delta == 0) {   // a reset
        reset();
        return;
    }
    if (!moving()) {
        return;
    }
    auto angles = t.distance(m_target > m_angle ? m_speed : -m_speed);
    if (std::abs(angles) >= std::abs(m_target - m_angle)) {
        m_angle = m_target;
    } else {
        m_angle += angles;
    }
    m_actors.activateBy(*this);
}

}
