#ifndef CANVAS_H_
#define CANVAS_H_

#include "defs.h"

class Canvas {
  public:
  void renderPose(cv::Mat& frame, Pose& pose, double r, double g, double b);
  static vector<pair<string, string>> pose_lines;
};

#endif
