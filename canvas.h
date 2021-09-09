#ifndef CANVAS_H
#define CANVAS_H

#include "defs.h"

class Canvas {
  public:
  void renderPose(cv::Mat& frame, Pose& pose);

  private:
  std::vector<std::pair<std::string, std::string>> pose_lines = 
    { // Face.
      {"left eye (outer)", "left eye"}, {"left eye", "left eye (inner)"}, {"left eye (inner)", "nose"},
      {"nose", "right eye (inner)"}, {"right eye (inner)", "right eye"}, {"right eye", "right eye (outer)"},
      {"mouth (right)", "mouth (left)"}, {"left eye (outer)", "left ear"}, {"right eye (outer)", "right ear"},
      // Torso.
      {"left shoulder", "right shoulder"}, {"left shoulder", "left hip"},
      {"right shoulder", "right hip"}, {"left hip", "right hip"},
      // Left arm.
      {"left shoulder", "left elbow"}, {"left elbow", "left wrist"}, {"left wrist", "left thumb"},
      {"left wrist", "left index"}, {"left index", "left pinky"}, {"left pinky", "left wrist"},
      // Right arm.
      {"right shoulder", "right elbow"}, {"right elbow", "right wrist"}, {"right wrist", "right thumb"},
      {"right wrist", "right index"}, {"right index", "right pinky"}, {"right pinky", "right wrist"},
      // Left leg.
      {"left hip", "left knee"}, {"left knee", "left ankle"},
      {"left ankle", "left foot index"}, {"left foot index", "left heel"}, {"left heel", "left ankle"},
      // Right leg.
      {"right hip", "right knee"}, {"right knee", "right ankle"}, 
      {"right ankle", "right foot index"}, {"right foot index", "right heel"}, {"right heel", "right ankle"},
    };
};

#endif
