#include "pose.h"

vector<string> PoseEstimator::body_parts = 
  { "nose", "right eye (inner)", "right eye", "right eye (outer)", "left eye (inner)", "left eye", "left eye (outer)", 
    "right ear", "left ear", "mouth (right)", "mouth (left)", "right shoulder", "left shoulder", "right elbow", "left elbow", 
    "right wrist", "left wrist", "right pinky", "left pinky", "right index", "left index", "right thumb", "left thumb", 
    "right hip", "left hip", "right knee", "left knee", "right ankle", "left ankle", "right heel", "left heel", "right foot index", "left foot index" };

void Pose::addLandmark(Landmark landmark) {
  landmarks.push_back(landmark);
}

Landmark Pose::getLandmark(string body_part) {
  vector<Landmark>::iterator itr =
    std::find_if(landmarks.begin(), landmarks.end(), [body_part](Landmark landmark) {
      return landmark.getBodyPart() == body_part; 
    });
  if (itr == landmarks.end()) {
    cerr << "ERROR: Invalid body part \"" << body_part << "\" in Pose::getLandmark()." << endl;
    exit(EXIT_FAILURE);
  }
  return *itr;
}

bool Pose::isEmpty() {
  return landmarks.empty();
}

Landmark::Landmark(string body_part, cv::Point3d position, double visibility, double presence) :
  body_part(body_part), position(position), visibility(visibility), presence(presence) {}

bool Landmark::isVisible() {
  return visibility > min_visibility;
}

string Landmark::getBodyPart() {
  return body_part;
}

cv::Point3d Landmark::getPosition() {
  return position;
}

double Landmark::getVisibility() {
  return visibility;
}

double Landmark::getPresence() {
  return presence;
}
