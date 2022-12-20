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
  // remove empty poses
  for (int i = player_poses.size() - 1; i >= 0; i--) {
    if (player_poses[i].empty() || avatar_poses[i].empty()) {
      player_poses.erase(player_poses.begin() + i);
      avatar_poses.erase(avatar_poses.begin() + i);
    }
  }
  if (player_poses.size() == 0) {
    return 0;
  }
  // denoise data
  denoise(player_poses);
  denoise(avatar_poses);
  // // standardize data
  for (Pose& player_pose : player_poses) {
    player_pose.standardize();
  }
  for (Pose& avatar_pose : avatar_poses) {
    avatar_pose.standardize();
  }
  // calculate score
  double total_cost = 0;
  for (int i = 0; i < player_poses.size(); i++) {
    double cost = M_PI * M_PI + offset_cost * player_poses.size() * player_poses.size();
    for (int j = 0; j < avatar_poses.size(); j++) {
      cost = std::min(cost, mse(player_poses[i], avatar_poses[j]) + offset_cost * (j - i) * (j - i));
    }
    total_cost += cost;
  }
  double mean_cost = total_cost / player_poses.size();
  double score = 100 * std::pow(sensitivity, mean_cost);
  return score;
}

void Scorer::denoise(vector<Pose>& poses) {
  vector<Pose> new_poses;
  for (int i = 1; i < poses.size() - 1; i++) {
    if (i == 0 || i == poses.size() - 1) {
      new_poses.push_back(poses[i]);
      continue;
    }
    Pose new_pose;
    for (string body_part : PoseEstimator::body_parts) {
      Landmark prev_landmark = poses[i - 1].getLandmark(body_part);
      Landmark curr_landmark = poses[i].getLandmark(body_part);
      Landmark next_landmark = poses[i + 1].getLandmark(body_part);
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
        double visibility = (prev_landmark.visibility + next_landmark.visibility) / 2;
        curr_landmark = Landmark(body_part, x, y, visibility);
      }
      new_pose.addLandmark(curr_landmark);
    }
    new_poses.push_back(new_pose);
  }
  poses = new_poses;
}

double Scorer::mse(Pose pose1, Pose pose2) {
  vector<string> body_parts = {"left wrist", "right wrist"};
  double total_distance;
  for (string body_part : body_parts) {
    Landmark landmark1 = pose1.getLandmark(body_part);
    Landmark landmark2 = pose2.getLandmark(body_part);
    double dx = landmark2.x - landmark1.x;
    double dy = landmark2.y - landmark1.y;
    total_distance += dx * dx + dy * dy;
  }
  double mean_distance = total_distance / body_parts.size();
  return mean_distance;
}

// vector<vector<string>> joints = {
//   {"left shoulder", "left elbow", "left wrist"}, {"right shoulder", "left shoulder", "left elbow"},
//   {"right shoulder", "right elbow", "right wrist"}, {"left shoulder", "right shoulder", "right elbow"},
// };
// double total_distance = 0;
// for (vector<string> joint : joints) {
//   Landmark player_landmark1 = player_pose.getLandmark(joint[0]);
//   Landmark player_landmark2 = player_pose.getLandmark(joint[1]);
//   Landmark player_landmark3 = player_pose.getLandmark(joint[2]);
//   Landmark avatar_landmark1 = avatar_pose.getLandmark(joint[0]);
//   Landmark avatar_landmark2 = avatar_pose.getLandmark(joint[1]);
//   Landmark avatar_landmark3 = avatar_pose.getLandmark(joint[2]);
//   if (!player_landmark1.visible() || !player_landmark2.visible() || !player_landmark3.visible()) {
//     total_distance += M_PI * M_PI;
//     continue;
//   }
//   if (!avatar_landmark1.visible() || !avatar_landmark2.visible() || !avatar_landmark3.visible()) {
//     continue;
//   }
//   double theta1 = angle(player_landmark1, player_landmark2, player_landmark3);
//   double theta2 = angle(avatar_landmark1, avatar_landmark2, avatar_landmark3);
//   total_distance += (theta1 - theta2) * (theta1 - theta2);
// }
// return total_distance / joints.size();

// double Scorer::angle(Landmark landmark1, Landmark landmark2, Landmark landmark3) {
//   double x1 = landmark1.x - landmark2.x;
//   double y1 = landmark1.y - landmark2.y;
//   double x2 = landmark3.x - landmark2.x;
//   double y2 = landmark3.y - landmark2.y;
//   double theta = std::acos((x1 * x2 + y1 * y2) / std::sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2)));
//   return theta;
// }
