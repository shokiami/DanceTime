#include "canvas.h"
#include "pose.h"

void Canvas::renderPose(cv::Mat& frame, Pose& pose, double r, double g, double b) {
  cv::Scalar color = cv::Scalar(b, g, r);
  for (pair<string, string> body_parts : pose_lines) {
    Landmark landmark_1 = pose.getLandmark(body_parts.first);
    Landmark landmark_2 = pose.getLandmark(body_parts.second);
    if (landmark_1.visible() && landmark_2.visible()) {
      cv::Point2d point1 = cv::Point2d(landmark_1.x, landmark_1.y);
      cv::Point2d point2 = cv::Point2d(landmark_2.x, landmark_2.y);
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
