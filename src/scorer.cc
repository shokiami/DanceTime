#include "scorer.h"

void Scorer::reset() {
  camera_poses.clear();
  video_poses.clear();
}

void Scorer::addPoses(Pose camera_pose, Pose video_pose) {
  camera_poses.push_back(camera_pose);
  video_poses.push_back(video_pose);
}

double Scorer::getScore() {
  if (camera_poses.size() != video_poses.size()) {
    ERROR("Number of poses does not match.");
  }
  // create copies
  vector<Pose> poses1 = camera_poses;
  vector<Pose> poses2 = video_poses;
  // erase empty poses
  for (int i = poses1.size() - 1; i >= 0; i--) {
    if (poses1[i].isEmpty() || poses2[i].isEmpty()) {
      poses1.erase(poses1.begin() + i);
      poses2.erase(poses2.begin() + i);
    }
  }
  if (poses1.size() == 0) {
    return 0;
  }
  // denoise data
  for (int i = 1; i < poses1.size() - 1; i++) {
    poses1[i] = filter(poses1[i - 1], poses1[i], poses1[i + 1]);
    poses2[i] = filter(poses2[i - 1], poses2[i], poses2[i + 1]);
  }
  // calculate score
  double total_similarity = 0;
  for (int i = 0; i < poses1.size() - 1; i++) {
    for (string body_part : PoseEstimator::body_parts) {
      Landmark curr_landmark = poses1[i].getLandmark(body_part);
      Landmark next_landmark = poses1[i + 1].getLandmark(body_part);
      double dx1 = next_landmark.x - curr_landmark.x;
      double dy1 = next_landmark.y - curr_landmark.y;
      curr_landmark = poses2[i].getLandmark(body_part);
      next_landmark = poses2[i + 1].getLandmark(body_part);
      double dx2 = next_landmark.x - curr_landmark.x;
      double dy2 = next_landmark.y - curr_landmark.y;
      total_similarity += (dx1 - dx2) * (dx1 - dx2) + (dy1 - dy2) * (dy1 - dy2);
    }
  }
  return total_similarity / (2 * PoseEstimator::body_parts.size() * poses1.size());
}

Pose Scorer::filter(Pose prev, Pose curr, Pose next) {
  Pose pose;
  for (string body_part : PoseEstimator::body_parts) {
    Landmark prev_landmark = prev.getLandmark(body_part);
    Landmark curr_landmark = curr.getLandmark(body_part);
    Landmark next_landmark = next.getLandmark(body_part);
    double dx1 = curr_landmark.x - prev_landmark.x;
    double dy1 = curr_landmark.y - prev_landmark.y;
    double dx2 = next_landmark.x - curr_landmark.x;
    double dy2 = next_landmark.y - curr_landmark.y;
    double dx3 = next_landmark.x - prev_landmark.x;
    double dy3 = next_landmark.y - prev_landmark.y;
    double d1 = dx1 * dx1 + dy1 + dy1;
    double d2 = dx2 * dx2 + dy2 + dy2;
    double d3 = dx3 * dx3 + dy3 + dy3;
    if (d3 < d1 || d3 < d2) {
      double x = (prev_landmark.x + next_landmark.x) / 2;
      double y = (prev_landmark.y + next_landmark.y) / 2;
      double z = (prev_landmark.z + next_landmark.z) / 2;
      double visibility = (prev_landmark.visibility + next_landmark.visibility) / 2;
      curr_landmark = Landmark(body_part, x, y, z, visibility);
    }
    pose.addLandmark(curr_landmark);
  }
  return pose;
}
