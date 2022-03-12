#ifndef CANVAS_H_
#define CANVAS_H_

#include "defs.h"

class Canvas {
  public:
  void renderPose(cv::Mat& frame, Pose& pose);
  static std::vector<std::pair<std::string, std::string>> pose_lines;
};

#endif
