#include "pose.h"
#include "canvas.h"

int main() {
  cv::VideoCapture capture(0);
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  while ((cv::waitKey(1) & 0xFF) != 'q') {
    capture.read(frame);
    if (!frame.empty()) {
      cv::flip(frame, frame, 1);
      Pose pose = pose_estimator.getPose(frame);
      canvas.renderPose(frame, pose);
      cv::imshow("DanceTime", frame);
    }
  }
  return 0;
}
