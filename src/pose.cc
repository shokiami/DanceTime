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
    ERROR("Trying to get landmark of empty pose.");
  }
  vector<Landmark>::iterator itr =
    std::find_if(landmarks.begin(), landmarks.end(), [body_part](Landmark landmark) {
      return landmark.body_part == body_part; 
    });
  if (itr == landmarks.end()) {
    ERROR("Invalid body part \"" + body_part + "\".");
  }
  return *itr;
}

bool Pose::empty() {
  return landmarks.empty();
}

void Pose::standardize() {
  Landmark nose = getLandmark("nose");
  Landmark left_shoulder = getLandmark("left shoulder");
  Landmark left_hip = getLandmark("left hip");
  Landmark right_shoulder = getLandmark("right shoulder");
  Landmark right_hip = getLandmark("right hip");
  double torso_height = ((left_hip.y - left_shoulder.y) + (right_hip.y - right_shoulder.y)) / 2;
  for (Landmark& landmark : landmarks) {
    landmark.x = (landmark.x - nose.x) / torso_height;
    landmark.y = (landmark.y - nose.y) / torso_height;
  }
}

void Pose::transform(double scalar, double dx, double dy) {
  for (Landmark& landmark : landmarks) {
    landmark.x = landmark.x * scalar + dx;
    landmark.y = landmark.y * scalar + dy;
  }
}

Landmark::Landmark(string body_part, double x, double y, double visibility) :
  body_part(body_part), x(x), y(y), visibility(visibility) {}

bool Landmark::visible() {
  return visibility > min_visibility;
}
