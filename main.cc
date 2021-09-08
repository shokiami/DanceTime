#include <thread>
#include "pose.h"
#include "canvas.h"

// class PoseThread {
//   public:
//   cv::Mat& frame;
//   Pose& pose;
//   PoseEstimator& pose_estimator;
//   std::mutex& frame_lock;
//   std::mutex& pose_lock;
//   bool& play;
//   void operator()() {
//     while (play) {
//       frame_lock.lock();
//       cv::Mat tmp_frame = frame;
//       frame_lock.unlock();
//       Pose tmp_pose = pose_estimator.estimate(tmp_frame);
//       if (!tmp_pose.landmarks.empty()) {
//         pose_lock.lock();
//         pose = tmp_pose;
//         pose_lock.unlock();
//       }
//     }
//   }
// };

int main() {
  cv::VideoCapture capture(0);
  cv::Mat frame;
  Pose pose;
  PoseEstimator pose_estimator;
  Canvas canvas;
  // std::mutex frame_lock;
  // std::mutex pose_lock;
  // bool play = true;
  // std::thread pose_thread(PoseThread{frame, pose, pose_estimator, frame_lock, pose_lock, play});
  while ((cv::waitKey(1) & 0xFF) != 'q') {
    capture.read(frame);
    if (!frame.empty()) {
      // frame_lock.lock();
      cv::flip(frame, frame, 1);
      Pose tmp_pose = pose_estimator.estimate(frame);
      if (!tmp_pose.landmarks.empty()) {
        pose = tmp_pose;
      }
      if (!pose.landmarks.empty()) {
        canvas.renderPose(frame, pose);
      }
      // if (!pose.landmarks.empty()) {
      //   // pose_lock.lock();
      //   canvas.renderPose(frame, pose);
      //   // pose_lock.unlock();
      // }
      cv::imshow("DanceTime", frame);
      // frame_lock.unlock();
    }
  }
  // play = false;
  // pose_thread.join();
  return 0;
}
