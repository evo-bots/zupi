#ifndef _SIM_CAMERA1D_H
#define _SIM_CAMERA1D_H

#include "servo.h"

namespace sim {

struct Pos1D {
    // by default distance/offset represents X/Y in cartesian coordinates
    // (suppose camera facing X positive is the original state - to be consistent
    // with converted polar coordinates)
    // after converted to polar coordinates, distance/offset is r/t, t is in
    // radians
    double distance;
    double offset;

    Pos1D(double d = 0, double o = 0)
        : distance(d), offset(o) {
    }

    // cartesian coordinates
    double x() const { return distance; }
    double y() const { return offset; }

    // polar coordinates
    double r() const { return distance; }
    double t() const { return offset; }

    Pos1D toPolars() const;
    Pos1D toCartesians() const;
};

struct Capturable1D : ::zupi::Interface {
    virtual Pos1D capture(const Pos1D&) const = 0;
};

class Camera1D : public Capturable1D {
public:
    static constexpr double F50 = 50;
    static constexpr double FILM35 = 35;

    Camera1D(double focal = F50, double film = FILM35);

    double focal() const { return m_focal; }
    Camera1D& focal(double f) { m_focal = f; return *this; }

    double film() const { return m_film; }
    Camera1D& film(double f) { m_film = f; return *this; }

    bool onFilm(const Pos1D&) const;

    // implement Capturable1D
    virtual Pos1D capture(const Pos1D&) const;

private:
    double m_focal; // focal length in mm
    double m_film;  // size of film in mm
};

class CaptureEnv1D : public Capturable1D {
public:
    static constexpr double ARM30 = 30;

    CaptureEnv1D(Camera1D *cam, double arm = ARM30, double angle = 0);

    Camera1D* camera() const { return m_camera; }

    double armLength() const { return m_arm; }
    CaptureEnv1D& armLength(double a) { m_arm = a; return *this; }

    double angle() const { return m_angle; }
    CaptureEnv1D& angle(double a) { m_angle = a; return *this; }

    // implement Capturable1D
    virtual Pos1D capture(const Pos1D&) const;

    Servo::Actor actor() const {
        return [this](const Servo& s) { angle(s.angle()); };
    }

private:
    Camera1D *m_camera;
    double m_arm;   // length of arm holding the camera, in mm
                    // the other end of the arm is the origin of rotation
    double m_angle; // current angle of camera, in radians, anti-clockwise is positive
};

}

#endif
