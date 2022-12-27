#include "canvas.h"

void Canvas::render(cv::Mat& frame, Pose& pose, double r, double g, double b) {
  cv::Scalar color = cv::Scalar(b, g, r);
  for (pair<string, string> body_parts : pose_lines) {
    if (pose.contains(body_parts.first) && pose.contains(body_parts.second)) {
      Point point1 = pose[body_parts.first];
      Point point2 = pose[body_parts.second];
      cv::line(frame, point1, point2, color, 3, cv::LINE_AA);
      cv::circle(frame, point1, 5, color, cv::FILLED, cv::LINE_AA);
      cv::circle(frame, point2, 5, color, cv::FILLED, cv::LINE_AA);
    }
  }
}

vector<pair<string, string>> Canvas::pose_lines = {
  // face
  {"left eye (outer)", "left eye"}, {"left eye", "left eye (inner)"}, {"left eye (inner)", "nose"}, 
  {"nose", "right eye (inner)"}, {"right eye (inner)", "right eye"}, {"right eye", "right eye (outer)"}, 
  {"mouth (right)", "mouth (left)"}, {"left eye (outer)", "left ear"}, {"right eye (outer)", "right ear"}, 
  // torso
  {"left shoulder", "right shoulder"}, {"left shoulder", "left hip"}, 
  {"right shoulder", "right hip"}, {"left hip", "right hip"}, 
  // left arm
  {"left shoulder", "left elbow"}, {"left elbow", "left wrist"}, {"left wrist", "left thumb"}, 
  {"left wrist", "left index"}, {"left index", "left pinky"}, {"left pinky", "left wrist"}, 
  // right arm
  {"right shoulder", "right elbow"}, {"right elbow", "right wrist"}, {"right wrist", "right thumb"}, 
  {"right wrist", "right index"}, {"right index", "right pinky"}, {"right pinky", "right wrist"}, 
  // left leg
  {"left hip", "left knee"}, {"left knee", "left ankle"}, 
  {"left ankle", "left foot index"}, {"left foot index", "left heel"}, {"left heel", "left ankle"}, 
  // right leg
  {"right hip", "right knee"}, {"right knee", "right ankle"}, 
  {"right ankle", "right foot index"}, {"right foot index", "right heel"}, {"right heel", "right ankle"}, 
};
