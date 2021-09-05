#include <thread>
#include "pose.h"
#include "canvas.h"

class PoseThread {
  public:
  cv::Mat& frame;
  Pose& pose;
  PoseEstimator& pose_estimator;
  std::mutex& frame_lock;
  std::mutex& pose_lock;
  bool& loop;
  void operator()() {
    while (loop) {
      frame_lock.lock();
      cv::Mat tmp_frame = frame;
      frame_lock.unlock();
      Pose tmp_pose = pose_estimator.estimate(tmp_frame);
      pose_lock.lock();
      pose = tmp_pose;
      pose_lock.unlock();
    }
  }
};

int main() {
  cv::VideoCapture videoCapture(0);
  cv::Mat frame;
  Pose pose;
  PoseEstimator pose_estimator;
  std::mutex frame_lock;
  std::mutex pose_lock;
  bool loop = true;
  Canvas canvas;
  videoCapture.read(frame);
  std::thread pose_thread(PoseThread{frame, pose, pose_estimator, frame_lock, pose_lock, loop});
  while ((cv::waitKey(1) & 0xFF) != 'q') {
    frame_lock.lock();
    videoCapture.read(frame);
    cv::flip(frame, frame, 1);
    frame_lock.unlock();
    if (!pose.landmarks.empty()) {
      pose_lock.lock();
      canvas.renderPose(frame, pose);
      pose_lock.unlock();
    }
    frame_lock.lock();
    imshow("DanceTime", frame);
    frame_lock.unlock();
  }
  loop = false;
  pose_thread.join();
  return 0;
}
