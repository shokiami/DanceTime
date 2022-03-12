#include "pose.h"
#include "canvas.h"
#include "video.h"

int main() {
  Video video("blackpink");
  Canvas canvas;
  video.play();
  while (!video.finished()) {
    cv::Mat frame = video.getFrame();
    Pose pose = video.getPose();
    canvas.renderPose(frame, pose);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
  }
  return EXIT_SUCCESS;
}

// cv::VideoCapture capture(0);
// cv::Mat frame;
// PoseEstimator pose_estimator;
// Canvas canvas;
// while ((cv::waitKey(1) & 0xFF) != 'q') {
//   capture.read(frame);
//   if (!frame.empty()) {
//     cv::flip(frame, frame, 1);
//     Pose pose = pose_estimator.getPose(frame);
//     canvas.renderPose(frame, pose);
//     cv::imshow("DanceTime", frame);
//   }
// }
// return EXIT_SUCCESS;
