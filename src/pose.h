#ifndef POSE_H_
#define POSE_H_

#include "defs.h"

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose(cv::Mat& frame, bool wait = false);
  static vector<string> body_parts;
};

class Pose {
  public:
  void addLandmark(Landmark landmark);
  Landmark getLandmark(string body_part);
  bool isEmpty();

  private:
  vector<Landmark> landmarks;
};

class Landmark {
  public:
  Landmark(string body_part, double x, double y, double z, double visibility);
  bool isVisible();
  string body_part;
  double x;
  double y;
  double z;
  double visibility;

  private:
  static constexpr double min_visibility = 0.1;
};

#endif
