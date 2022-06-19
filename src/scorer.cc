#include "scorer.h"

void Scorer::reset() {
  player_poses.clear();
  avatar_poses.clear();
}

void Scorer::addPoses(Pose player_pose, Pose avatar_pose) {
  player_poses.push_back(player_pose);
  avatar_poses.push_back(avatar_pose);
}

double Scorer::getScore() {
  if (player_poses.size() != avatar_poses.size()) {
    ERROR("Number of poses does not match.");
  }
  // create copies
  vector<Pose> poses1 = player_poses;
  vector<Pose> poses2 = avatar_poses;
  // remove empty poses
  for (int i = poses1.size() - 1; i >= 0; i--) {
    if (poses1[i].empty() || poses2[i].empty()) {
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
  double score = 0;
  double max_score = 0;
  for (int i = 0; i < poses1.size() - 1; i++) {
    for (string body_part : PoseEstimator::body_parts) {
      Landmark curr1 = poses1[i].getLandmark(body_part);
      Landmark next1 = poses1[i + 1].getLandmark(body_part);
      Landmark curr2 = poses2[i].getLandmark(body_part);
      Landmark next2 = poses2[i + 1].getLandmark(body_part);
      if (curr1.visible() && next1.visible() && curr2.visible() && next2.visible()) {
        double dx1 = next1.x - curr1.x;
        double dy1 = next1.y - curr1.y;
        double d1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
        double dx2 = next2.x - curr2.x;
        double dy2 = next2.y - curr2.y;
        double d2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
        double dot = dx1 * dx2 + dy1 * dy2;
        if (dot > 0) {
          score += dot;
        }
        max_score += d1 * d2;
      }
    }
  }
  return score / max_score;
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
