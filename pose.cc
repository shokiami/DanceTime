#include "pose.h"

Landmark Pose::get(std::string body_part) {
  return landmarks.at(body_part);
}

void Pose::add(Landmark& landmark) {
  landmarks[landmark.body_part] = landmark;
}

bool Landmark::isVisible() {
  return visibility > min_visibility && presence > min_presence;
}

cv::Point2d Landmark::framePosition(cv::Mat& frame) {
  int frame_width = frame.size[1];
  int frame_height = frame.size[0];
  return cv::Point2d(position.x * frame_width, position.y * frame_height);
}

std::ostream& operator << (std::ostream& out, Landmark& obj) {
  out << "body_part: " << obj.body_part << ", ";
  out << "position: {" << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << "}, ";
  out << "visibility: " << obj.visibility << ", ";
  out << "presence: " << obj.presence;
  return out;
}
