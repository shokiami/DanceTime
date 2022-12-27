#ifndef POSE_H_
#define POSE_H_

#include "defs.h"
#include "util.h"

typedef Map<string, Point> Pose;

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose(cv::Mat& frame, bool wait = false);
  static vector<string> body_parts;
};

#endif
