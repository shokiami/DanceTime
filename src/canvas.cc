#include "canvas.h"

const vector<pair<string, string>> Canvas::pose_lines = {
  // face
  {"left_eye_outer", "left_eye"}, {"left_eye", "left_eye_inner"}, {"left_eye_inner", "nose"}, 
  {"nose", "right_eye_inner"}, {"right_eye_inner", "right_eye"}, {"right_eye", "right_eye_outer"}, 
  {"mouth_right", "mouth_left"}, {"left_eye_outer", "left_ear"}, {"right_eye_outer", "right_ear"}, 
  // torso
  {"left_shoulder", "right_shoulder"}, {"left_shoulder", "left_hip"}, 
  {"right_shoulder", "right_hip"}, {"left_hip", "right_hip"}, 
  // left_arm
  {"left_shoulder", "left_elbow"}, {"left_elbow", "left_wrist"}, {"left_wrist", "left_thumb"}, 
  {"left_wrist", "left_index"}, {"left_index", "left_pinky"}, {"left_pinky", "left_wrist"}, 
  // right_arm
  {"right_shoulder", "right_elbow"}, {"right_elbow", "right_wrist"}, {"right_wrist", "right_thumb"}, 
  {"right_wrist", "right_index"}, {"right_index", "right_pinky"}, {"right_pinky", "right_wrist"}, 
  // left_leg
  {"left_hip", "left_knee"}, {"left_knee", "left_ankle"}, 
  {"left_ankle", "left_foot index"}, {"left_foot index", "left_heel"}, {"left_heel", "left_ankle"}, 
  // right_leg
  {"right_hip", "right_knee"}, {"right_knee", "right_ankle"}, 
  {"right_ankle", "right_foot_index"}, {"right_foot_index", "right_heel"}, {"right_heel", "right_ankle"}, 
};

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
