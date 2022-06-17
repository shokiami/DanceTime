#ifndef CANVAS_H_
#define CANVAS_H_

#include "defs.h"

class Canvas {
  public:
  void renderPose(cv::Mat& frame, Pose& pose, cv::Scalar color);
  static vector<pair<string, string>> pose_lines;
};

#endif
