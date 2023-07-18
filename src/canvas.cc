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

void Canvas::render_pose(cv::Mat& frame, Pose& pose, int r, int g, int b) {
  for (pair<string, string> body_parts : pose_lines) {
    if (pose.contains(body_parts.first) && pose.contains(body_parts.second)) {
      Point point1 = pose[body_parts.first];
      Point point2 = pose[body_parts.second];
      cv::line(frame, point1, point2, CV_RGB(r, g, b), 3, cv::LINE_AA);
      cv::circle(frame, point1, 5, CV_RGB(r, g, b), cv::FILLED, cv::LINE_AA);
      cv::circle(frame, point2, 5, CV_RGB(r, g, b), cv::FILLED, cv::LINE_AA);
    }
  }
}

void Canvas::render_text(cv::Mat& frame, string text, double x, double y, int r, int g, int b) {
  cv::putText(frame, text, Point(x, y), 0, 1.0, CV_RGB(r, g, b), 2, cv::LINE_AA);
}

void Canvas::render_score(cv::Mat& frame, int score, double progress, int r, int g, int b, bool left) {
  double x = 8.0;
  if (!left) {
    x = frame.cols - 9.0;
  }
  double y = (1.0 - progress) * frame.rows;
  cv::line(frame, Point(x, frame.rows), Point(x, y), CV_RGB(r, g, b), 15, cv::LINE_AA);
  if (left) {
    x += 10.0;
  } else {
    x -= 60.0;
  }
  y -= 20.0;
  render_text(frame, std::to_string(score) + "%", x, y, r, g, b);
}
