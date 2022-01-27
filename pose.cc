#include "pose.h"

void Pose::addLandmark(Landmark landmark) {
  landmarks.push_back(landmark);
}

Landmark Pose::getLandmark(std::string body_part) {
  std::vector<Landmark>::iterator itr =
    std::find_if(landmarks.begin(), landmarks.end(), [body_part](Landmark landmark) {
      return landmark.getBodyPart() == body_part; 
    });
  if (itr == landmarks.end()) {
    std::cerr << "ERROR: Invalid body part \"" << body_part << "\" in Pose::getLandmark()." << std::endl;
    exit(EXIT_FAILURE);
  }
  return *itr;
}

bool Pose::isEmpty() {
  return landmarks.empty();
}

Landmark::Landmark(std::string body_part, cv::Point3d position, double visibility, double presence) :
  body_part(body_part), position(position), visibility(visibility), presence(presence) {}

bool Landmark::isVisible() {
  return visibility > min_visibility;
}

cv::Point2d Landmark::framePosition(cv::Mat& frame) {
  const size_t frame_width = frame.size[1];
  const size_t frame_height = frame.size[0];
  return cv::Point2d(position.x * frame_width, position.y * frame_height);
}

std::string Landmark::getBodyPart() {
  return body_part;
}

cv::Point3d Landmark::getPosition() {
  return position;
}

double Landmark::getVisibility() {
  return visibility;
}

double Landmark::getPresence() {
  return presence;
}
