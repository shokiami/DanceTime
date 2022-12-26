#include "pose.h"

vector<string> PoseEstimator::body_parts = {
  "nose", "right eye (inner)", "right eye", "right eye (outer)", "left eye (inner)", "left eye", "left eye (outer)", 
  "right ear", "left ear", "mouth (right)", "mouth (left)", "right shoulder", "left shoulder", "right elbow", "left elbow", 
  "right wrist", "left wrist", "right pinky", "left pinky", "right index", "left index", "right thumb", "left thumb", 
  "right hip", "left hip", "right knee", "left knee", "right ankle", "left ankle", "right heel", "left heel", "right foot index", "left foot index"
};

void Pose::addLandmark(Landmark landmark) {
  landmarks.push_back(landmark);
}

Landmark Pose::getLandmark(string body_part) {
  if (empty()) {
    ERROR("trying to get landmark of empty pose");
  }
  vector<Landmark>::iterator itr =
    std::find_if(landmarks.begin(), landmarks.end(), [body_part](Landmark landmark) {
      return landmark.body_part == body_part; 
    });
  if (itr == landmarks.end()) {
    ERROR("invalid body part \"" + body_part + "\"");
  }
  return *itr;
}

bool Pose::empty() {
  return landmarks.empty();
}

void Pose::standardize() {
  Landmark left_shoulder = getLandmark("left shoulder");
  Landmark left_hip = getLandmark("left hip");
  Landmark right_shoulder = getLandmark("right shoulder");
  Landmark right_hip = getLandmark("right hip");
  double center_x = (left_shoulder.x + left_hip.x + right_shoulder.x + right_hip.x) / 4;
  double center_y = (left_shoulder.y + left_hip.y + right_shoulder.y + right_hip.y) / 4;
  double torso_height = ((left_hip.y - left_shoulder.y) + (right_hip.y - right_shoulder.y)) / 2;
  for (Landmark& landmark : landmarks) {
    landmark.x = (landmark.x - center_x) / torso_height;
    landmark.y = (landmark.y - center_y) / torso_height;
  }
}

void Pose::transform(double scalar, double x_diff, double y_diff) {
  for (Landmark& landmark : landmarks) {
    landmark.x = landmark.x * scalar + x_diff;
    landmark.y = landmark.y * scalar + y_diff;
  }
}

Landmark::Landmark(string body_part, double x, double y, double visibility) :
  body_part(body_part), x(x), y(y), visibility(visibility) {}

bool Landmark::visible() {
  return visibility > min_visibility;
}
