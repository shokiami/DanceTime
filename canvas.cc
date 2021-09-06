#include "canvas.h"
#include "pose.h"

void Canvas::renderPose(cv::Mat& frame, Pose pose) {
  for (std::pair<std::string, std::string> body_parts : pose_lines) {
    Landmark landmark_1 = pose.get(body_parts.first);
    Landmark landmark_2 = pose.get(body_parts.second);
    if (landmark_1.presence > min_presence && landmark_2.presence > min_presence) {
      cv::line(frame, landmark_1.framePosition(frame), landmark_2.framePosition(frame), cv::Scalar(75, 125, 255), 3, cv::LINE_AA);
      cv::circle(frame, landmark_1.framePosition(frame), 5, cv::Scalar(75, 125, 255), cv::FILLED, cv::LINE_AA);
      cv::circle(frame, landmark_2.framePosition(frame), 5, cv::Scalar(75, 125, 255), cv::FILLED, cv::LINE_AA);
    }
  }
}
