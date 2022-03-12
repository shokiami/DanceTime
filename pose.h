#ifndef POSE_H_
#define POSE_H_

#include "defs.h"

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose(cv::Mat& frame, bool wait = false);
  std::vector<std::string> getBodyParts();

  private:
  std::vector<std::string> body_parts = 
    { "nose", "right eye (inner)", "right eye", "right eye (outer)", "left eye (inner)", "left eye", "left eye (outer)", 
      "right ear", "left ear", "mouth (right)", "mouth (left)", "right shoulder", "left shoulder", "right elbow", "left elbow", 
      "right wrist", "left wrist", "right pinky", "left pinky", "right index", "left index", "right thumb", "left thumb", 
      "right hip", "left hip", "right knee", "left knee", "right ankle", "left ankle", "right heel", "left heel", "right foot index", "left foot index" };
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
  cv::Point2d framePosition(cv::Mat& frame);
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
