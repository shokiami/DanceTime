#include "canvas.h"
#include "pose.h"

void Canvas::renderPose(cv::Mat& frame, Pose& pose) {
  for (std::pair<std::string, std::string> body_parts : pose_lines) {
    Landmark landmark_1 = pose.getLandmark(body_parts.first);
    Landmark landmark_2 = pose.getLandmark(body_parts.second);
    if (landmark_1.isVisible() && landmark_2.isVisible()) {
      cv::line(frame, landmark_1.framePosition(frame), landmark_2.framePosition(frame), cv::Scalar(255, 125, 75), 3, cv::LINE_AA);
      cv::circle(frame, landmark_1.framePosition(frame), 5, cv::Scalar(255, 125, 75), cv::FILLED, cv::LINE_AA);
      cv::circle(frame, landmark_2.framePosition(frame), 5, cv::Scalar(255, 125, 75), cv::FILLED, cv::LINE_AA);
    }
  }
}
