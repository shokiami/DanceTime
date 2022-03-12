#include "pose.h"
#include "canvas.h"
#include "video.h"

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
  return EXIT_SUCCESS;

  // cv::VideoCapture capture(0);
  // PoseEstimator pose_estimator;
  // Canvas canvas;

  // Video video("blackpink");
  // video.play();

  // while (!video.finished()) {
  //   cv::Mat camera_frame;
  //   capture.read(camera_frame);
  //   if (!camera_frame.empty()) {
  //     cv::flip(camera_frame, camera_frame, 1);
  //     Pose camera_pose = pose_estimator.getPose(camera_frame);
  //     canvas.renderPose(camera_frame, camera_pose);

  //     // cv::Mat video_frame;
  //     // video_frame = video.getFrame();
  //     // Pose video_pose = video.getPose();
  //     // canvas.renderPose(video_frame, video_pose);

  //     // cv::hconcat(video_frame, camera_frame);
  //     cv::imshow("DanceTime", camera_frame);
  //     cv::waitKey(1);
  //   }
  // }
  // return EXIT_SUCCESS;
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
