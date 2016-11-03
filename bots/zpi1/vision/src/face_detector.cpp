#include "face_detector.h"

using namespace std;
using namespace cv;
using namespace vision;

FaceDetector::FaceDetector(const string& dir, bool lbp, double scaleFactor, int minNeighbors, int minSize)
: m_frontalface("face", dir + (lbp ? "lbpcascades/lbpcascade_frontalface.xml" : "haarcascades/haarcascade_frontalface_default.xml")),
  m_eyes("eyes", dir + "haarcascades/haarcascade_eye_tree_eyeglasses.xml") {
      m_frontalface.scaleFactor(scaleFactor);
      m_frontalface.minNeighbors(minNeighbors);
      m_frontalface.minSize(Size(minSize, minSize));
}

size_t FaceDetector::detect(const Mat& image, DetectedObjectList& objects) {
    size_t count = 0;
    DetectResult result(&m_frontalface, image);
    for (auto& obj : result.objects) {
        cout << obj.json() << endl;
        auto subImg = image(obj.rc);
        if (!DetectResult(&m_eyes, subImg).empty()) {
            objects.push_back(obj);
            count ++;
        }
    }
    return count;
}
