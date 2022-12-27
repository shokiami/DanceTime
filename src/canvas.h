#ifndef CANVAS_H_
#define CANVAS_H_

#include "defs.h"
#include "pose.h"

class Canvas {
  public:
  void render(cv::Mat& frame, Pose& pose, double r, double g, double b);
  static vector<pair<string, string>> pose_lines;
};

#endif
