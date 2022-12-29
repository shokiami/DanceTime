#include "scorer.h"
#include <eigen3/Eigen/Dense>
#include <float.h>

const vector<string> Scorer::body_parts = {"left_wrist", "right_wrist"};

void Scorer::reset() {
  player_poses.clear();
  avatar_poses.clear();
}

void Scorer::add(Pose player_pose, Pose avatar_pose) {
  player_poses.push_back(player_pose);
  avatar_poses.push_back(avatar_pose);
}

double Scorer::score() {
  if (player_poses.size() != avatar_poses.size()) {
    ERROR("number of poses does not match");
  }
  vector<Pose> player_poses_copy = player_poses;
  vector<Pose> avatar_poses_copy = avatar_poses;
  // remove outliers
  remove_outliers(player_poses_copy);
  remove_outliers(avatar_poses_copy);
  // standardize data
  standardize(player_poses_copy);
  standardize(avatar_poses_copy);
  // compute polynomial approximations to data
  Polys player_polys = fit(player_poses_copy);
  Polys avatar_polys = fit(avatar_poses_copy);
  // if avatar is out of frame, return score of 1
  if (avatar_polys.empty()) {
    return 1;
  }
  // if player is out of frame, return score of 0
  if (player_polys.empty()) {
    return 0;
  }
  // scan for offset which yields the best max error
  double t_elapsed = player_poses_copy.size();
  double best_error = DBL_MAX;
  double best_offset = 0;
  for (double t_offset = -0.5 * t_elapsed; t_offset <= 0.5 * t_elapsed; t_offset += resolution) {
    double error = max_error(player_polys, avatar_polys, 0, t_elapsed, t_offset);
    if (error < best_error || (error == best_error && std::abs(t_offset) < std::abs(best_offset))) {
      best_error = error;
      best_offset = t_offset;
    }
  }
  // calculate score from best error and offset
  return error_to_score(best_error + offset_cost * std::abs(best_offset));
}

double Scorer::error_to_score(double error) {
  return 1 / (1 + std::pow(1000, error - midpoint));
}

void Scorer::remove_outliers(vector<Pose>& poses) {
  vector<Pose> new_poses;
  for (int i = 0; i < poses.size(); i++) {
    if (i == 0 || i == poses.size() - 1) {
      new_poses.push_back(poses[i]);
      continue;
    }
    Pose new_pose;
    for (string body_part : poses[i].keys()) {
      Point curr_point = poses[i][body_part];
      new_pose[body_part] = curr_point;
      if (!poses[i - 1].contains(body_part) || !poses[i + 1].contains(body_part)) {
        continue;
      }
      Point prev_point = poses[i - 1][body_part];
      Point next_point = poses[i + 1][body_part];
      double adj1_x_diff = curr_point.x - prev_point.x;
      double adj1_y_diff = curr_point.y - prev_point.y;
      double adj2_x_diff = next_point.x - curr_point.x;
      double adj2_y_diff = next_point.y - curr_point.y;
      double ops_x_diff = next_point.x - prev_point.x;
      double ops_y_diff = next_point.y - prev_point.y;
      double adj1_sq_dist = adj1_x_diff * adj1_x_diff + adj1_y_diff * adj1_y_diff;
      double adj2_sq_dist = adj2_x_diff * adj2_x_diff + adj2_y_diff * adj2_y_diff;
      double ops_sq_dist = ops_x_diff * ops_x_diff + ops_y_diff * ops_y_diff;
      if (ops_sq_dist < adj1_sq_dist || ops_sq_dist < adj2_sq_dist) {
        new_pose[body_part] = (prev_point + next_point) / 2;
      }
    }
    new_poses.push_back(new_pose);
  }
  poses = new_poses;
}

void Scorer::standardize(vector<Pose>& poses) {
  for (Pose& pose : poses) {
    if (!pose.contains("left_shoulder") || !pose.contains("left_hip") ||
        !pose.contains("right_shoulder") || !pose.contains("right_hip")) {
      pose = Pose();
      continue;
    }
    Point left_shoulder = pose["left_shoulder"];
    Point left_hip = pose["left_hip"];
    Point right_shoulder = pose["right_shoulder"];
    Point right_hip = pose["right_hip"];
    Point center = (left_shoulder + left_hip + right_shoulder + right_hip) / 4;
    double torso_height = ((left_hip.y - left_shoulder.y) + (right_hip.y - right_shoulder.y)) / 2;
    for (string body_part : pose.keys()) {
      Point& point = pose[body_part];
      point = (point - center) / torso_height;
    }
  }
}

Polys Scorer::fit(vector<Pose>& poses) {
  vector<int> idxs;
  for (int i = 0; i < poses.size(); i++) {
    bool contains_all = true;
    for (string body_part : body_parts) {
      if (!poses[i].contains(body_part)) {
        contains_all = false;
        break;
      }
    }
    if (contains_all) {
      idxs.push_back(i);
    }
  }
  if (idxs.size() <= poly_degree) {
    return Polys();
  }
  Eigen::MatrixXd a = Eigen::MatrixXd(idxs.size(), poly_degree + 1);
  for (int i = 0; i < idxs.size(); i++) {
    int idx = idxs[i];
    for (int j = 0; j <= poly_degree; j++) {
      int d = poly_degree - j;
      a(i, j) = std::pow(idx, d);
    }
  }
  Eigen::MatrixXd p = a * (a.transpose() * a).inverse() * a.transpose();
  Polys polys;
  for (string body_part : body_parts) {
    Eigen::VectorXd b_x = Eigen::VectorXd(idxs.size());
    Eigen::VectorXd b_y = Eigen::VectorXd(idxs.size());
    for (int i = 0; i < idxs.size(); i++) {
      int idx = idxs[i];
      Point point = poses[idx][body_part];
      b_x(i) = point.x;
      b_y(i) = point.y;
    }
    Eigen::VectorXd p_x = p * b_x;
    Eigen::VectorXd p_y = p * b_y;
    Eigen::VectorXd eigen_x_coeffs = a.colPivHouseholderQr().solve(p_x);
    Eigen::VectorXd eigen_y_coeffs = a.colPivHouseholderQr().solve(p_y);
    Coeffs x_coeffs = Coeffs(eigen_x_coeffs.data(), eigen_x_coeffs.data() + eigen_x_coeffs.size());
    Coeffs y_coeffs = Coeffs(eigen_y_coeffs.data(), eigen_y_coeffs.data() + eigen_y_coeffs.size());
    polys[body_part] = {x_coeffs, y_coeffs};
  }
  return polys;
}

double Scorer::max_error(Polys player_polys, Polys avatar_polys, double t_start, double t_end, double t_offset) {
  if (t_start >= t_end) {
    ERROR("end time must be greater than start time");
  }
  double total_error = 0;
  int num_samples = 0;
  for (string body_part : body_parts) {
    Coeffs player_x_coeffs = player_polys[body_part].first;
    Coeffs player_y_coeffs = player_polys[body_part].second;
    Coeffs avatar_x_coeffs = avatar_polys[body_part].first;
    Coeffs avatar_y_coeffs = avatar_polys[body_part].second;
    double max_error = 0;
    for (double t = t_start; t < t_end; t += resolution) {
      double x_diff = evaluate(avatar_x_coeffs, t) - evaluate(player_x_coeffs, t - t_offset);
      double y_diff = evaluate(avatar_y_coeffs, t) - evaluate(player_y_coeffs, t - t_offset);
      double error = x_diff * x_diff + y_diff * y_diff;
      if (error > max_error) {
        max_error = error;
      }
    }
    total_error += max_error;
    num_samples++;
  }
  return total_error / num_samples;
}

double Scorer::evaluate(Coeffs coeffs, double t) {
  double result = 0;
  int degree = coeffs.size() - 1;
  for (int i = 0; i < coeffs.size(); i++) {
    int d = degree - i;
    result += coeffs[i] * std::pow(t, d);
  }
  return result;
}
