#ifndef POSE_H_
#define POSE_H_

#include "defs.h"

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose(cv::Mat& frame, bool wait = false);
  static std::vector<std::string> body_parts;
};

class Pose {
  public:
  void addLandmark(Landmark landmark);
  Landmark getLandmark(std::string body_part);
  bool isEmpty();

  private:
  std::vector<Landmark> landmarks;
};

class Landmark {
  public:
  Landmark(std::string body_part, cv::Point3d position, double visibility, double presence);
  bool isVisible();
  std::string getBodyPart();
  cv::Point3d getPosition();
  double getVisibility();
  double getPresence();

  private:
  std::string body_part;
  cv::Point3d position;
  double visibility;
  double presence;
  static constexpr double min_visibility = 0.1;
};

#endif
