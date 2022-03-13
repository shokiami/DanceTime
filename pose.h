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
  Landmark(string body_part, cv::Point3d position, double visibility, double presence);
  bool isVisible();
  string getBodyPart();
  cv::Point3d getPosition();
  double getVisibility();
  double getPresence();

  private:
  string body_part;
  cv::Point3d position;
  double visibility;
  double presence;
  static constexpr double min_visibility = 0.1;
};

#endif
