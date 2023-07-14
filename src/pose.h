#ifndef POSE_H_
#define POSE_H_

#include "defs.h"
#include "util.h"

typedef Map<string, Point> Pose;

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose1(cv::Mat& frame, bool wait = false);
  Pose getPose2(cv::Mat& frame, bool wait = false);
  static const vector<string> body_parts;
};

#endif
