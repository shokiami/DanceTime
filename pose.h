#ifndef POSE_H
#define POSE_H

#include "defs.h"

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose estimate(cv::Mat& frame);

  private:
  std::vector<std::string> body_parts = 
    { "nose", "right eye (inner)", "right eye", "right eye (outer)", "left eye (inner)", "left eye", "left eye (outer)", 
      "right ear", "left ear", "mouth (right)", "mouth (left)", "right shoulder", "left shoulder", "right elbow", "left elbow", 
      "right wrist", "left wrist", "right pinky", "left pinky", "right index", "left index", "right thumb", "left thumb", 
      "right hip", "left hip", "right knee", "left knee", "right ankle", "left ankle", "right heel", "left heel", "right foot index", "left foot index" };
};

class Pose {
  public:
  Landmark get(std::string body_part);
  void add(Landmark landmark);
  std::map<std::string, Landmark> landmarks;
};

class Landmark {
  public:
  cv::Point2d framePosition(cv::Mat frame);
  friend std::ostream& operator << (std::ostream& out, Landmark& obj);
  std::string body_part;
  cv::Point3d position;
  double visibility;
  double presence;
};

#endif
