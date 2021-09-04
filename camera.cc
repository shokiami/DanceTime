#include "camera.h"
#include "pose.h"
#include "canvas.h"

int Camera::main()
{
  cv::VideoCapture videoCapture(0);
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  while ((cv::waitKey(1) & 0xFF) != 'q') {
    videoCapture.read(frame);
    cv::flip(frame, frame, 1);
    Pose pose = pose_estimator.estimate(frame);
    canvas.renderPose(frame, pose);
    imshow("DanceTime", frame);
  }
  return 0;
}