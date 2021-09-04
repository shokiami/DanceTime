#include "pose.h"

Landmark Pose::get(std::string body_part) {
  return landmarks.at(body_part);
}

void Pose::add(Landmark landmark) {
  landmarks[landmark.body_part] = landmark;
}

cv::Point Landmark::framePosition(cv::Mat frame) {
  int frame_width = frame.size[1];
  int frame_height = frame.size[0];
  return cv::Point(position.x * frame_width, position.y * frame_height);
}

std::ostream& operator << (std::ostream& out, Landmark& obj) {
  out << "body_part: " << obj.body_part << ", ";
  out << "position: {" << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << "}, ";
  out << "visibility: " << obj.visibility << ", ";
  out << "presence: " << obj.presence;
  return out;
}
