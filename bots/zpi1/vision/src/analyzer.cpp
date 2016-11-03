#include "analyzer.h"

using namespace std;
using namespace cv;
using namespace vision;

Analyzer::Analyzer(const string& dir)
: m_smile("smile", dir + "haarcascades/haarcascade_smile.xml"),
  m_face_def("face", dir + "haarcascades/haarcascade_frontalface_default.xml"),
  m_face_alt("face", dir + "haarcascades/haarcascade_frontalface_alt.xml"),
  m_face_alt2("face", dir + "haarcascades/haarcascade_frontalface_alt.xml"),
  m_face_pro("face", dir + "haarcascades/haarcascade_profileface.xml"),
  m_body_full("body", dir + "haarcascades/haarcascade_fullbody.xml"),
  m_body_upper("body", dir + "haarcascades/haarcascade_upperbody.xml"),
  m_body_lower("body", dir + "haarcascades/haarcascade_lowerbody.xml") {
    m_faces.add(&m_face_def); m_face_def.add(&m_smile);
    m_faces.add(&m_face_alt); m_face_alt.add(&m_smile);
    m_faces.add(&m_face_alt2); m_face_alt2.add(&m_smile);
    m_faces.add(&m_face_pro);
    m_bodies.add(&m_body_full);
    m_bodies.add(&m_body_upper);
    m_bodies.add(&m_body_lower);
    m_models.add(&m_faces);
    m_models.add(&m_bodies);
}

size_t Analyzer::detect(const Mat& image, DetectedObjectList& objects) {
    return m_models.detect(image, objects);
}
