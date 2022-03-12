#include "pose.h"
#include "canvas.h"
#include "video.h"

int main() {
  Video video("blackpink");

  cv::VideoCapture capture(0);
  PoseEstimator pose_estimator;
  Canvas canvas;

  video.play();

  while (!video.finished()) {
    cv::Mat camera_frame;
    capture.read(camera_frame);
    if (!camera_frame.empty()) {
      cv::flip(camera_frame, camera_frame, 1);
      Pose camera_pose = pose_estimator.getPose(camera_frame);
      canvas.renderPose(camera_frame, camera_pose);
      int height = camera_frame.size[0];
      int width = camera_frame.size[1];
      int target_width = video.getWidth() * height / video.getHeight();
      camera_frame = camera_frame(cv::Range(0, height),
        cv::Range(width / 2 - target_width / 2, width / 2 + target_width / 2));

      cv::Mat video_frame;
      video_frame = video.getFrame();
      Pose video_pose = video.getPose();
      canvas.renderPose(video_frame, video_pose);
      resize(video_frame, video_frame, cv::Size(target_width, height));

      cv::Mat frame;
      cv::hconcat(video_frame, camera_frame, frame);
      cv::imshow("DanceTime", frame);
      cv::waitKey(1);
    }
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
