#ifndef CANVAS_H_
#define CANVAS_H_

#include "defs.h"
#include "pose.h"

class Canvas {
  public:
  void render_pose(cv::Mat& frame, Pose& pose, int r, int g, int b);
  void render_text(cv::Mat& frame, string text, double x, double y, int r, int g, int b);
  void render_score(cv::Mat& frame, int score, double progress, int r, int g, int b, bool left);
  static const vector<pair<string, string>> pose_lines;
};

#endif
