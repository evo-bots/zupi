#define _USE_MATH_DEFINES
#include <cmath>
#include "camera1d.h"

namespace sim {

Pos1D Pos1D::toPolars() const {
    Pos1D p(std::sqrt(x()*x() + y()*y()));
    if (x() != 0) {
        p.offset = std::atanh(y()/x());
    } else if (y() > 0) {
        p.offset = M_PI_2;
    } else if (y() < 0) {
        p.offset = M_PI + M_PI_2;
    } else {
        p.offset = 0;
    }
    return p;
}

Pos1D Pos1D::toCartesians() const {
    return Pos1D(distance * std::cos(offset), distance * std::sin(offset));
}

Camera1D::Camera1D(double focal /* = F50 */, double film /* = FILM35 */)
: m_focal(focal), m_film(film) {

}

Pos1D Camera1D::capture(const Pos1D& pos) const {
    Pos1D p;
    if (pos.distance <= m_focal) {
        p.distance = -1;
    } else if (pos.offset != 0) {
        p.offset = pos.offset * m_focal / (pos.distance - m_focal);
        p.distance = m_focal * p.offset / pos.offset + m_focal;
    }
    return p;
}

bool Camera1D::onFilm(const Pos1D& pos) const {
    if (pos.distance <= 0) {
        return false;
    }
    return std::abs(pos.offset) < m_film / 2;
}

CaptureEnv1D::CaptureEnv1D(Camera1D *cam, double arm /* = ARM30 */, double angle /* = 0 */)
: m_camera(cam), m_arm(arm), m_angle(angle) {

}

Pos1D CaptureEnv1D::capture(const Pos1D& pos) const {
    auto absPos = pos.toPolars();
    // map to camera
    auto diffAngle = m_angle - absPos.t();
    auto camPos = Pos1D(absPos.r()*std::cos(diffAngle) - m_arm,
        absPos.r()*std::sin(diffAngle));
    return m_camera->capture(camPos);
}

}
