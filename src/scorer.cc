#include "scorer.h"
#include <eigen3/Eigen/Dense>
#include <float.h>

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
  // standardize data
  standardize(player_poses_copy);
  standardize(avatar_poses_copy);
  // compute polynomial approximations to data
  Polys player_polys = fit(player_poses_copy);
  Polys avatar_polys = fit(avatar_poses_copy);
  // scan for best error and offset
  double t_elapsed = player_poses.size();
  double best_error = DBL_MAX;
  double best_offset = 0;
  for (double t_offset = -t_elapsed; t_offset <= t_elapsed; t_offset += resolution) {
    double error = weighted_mse(player_polys, avatar_polys, 0, t_elapsed, t_offset);
    if (error < best_error) {
      best_error = error;
      best_offset = t_offset;
    }
  }
  // calculate score
  double score = 100 * std::pow(sensitivity, best_error + offset_cost * best_offset * best_offset);
  score = 0.01 * std::round(100 * score);
  return score;
}

void Scorer::standardize(vector<Pose>& poses) {
  for (Pose& pose : poses) {
    if (!pose.contains("left shoulder") || !pose.contains("left hip") ||
        !pose.contains("right shoulder") || !pose.contains("right hip")) {
      pose = Pose();
      continue;
    }
    Point left_shoulder = pose["left shoulder"];
    Point left_hip = pose["left hip"];
    Point right_shoulder = pose["right shoulder"];
    Point right_hip = pose["right hip"];
    double center_x = (left_shoulder.x + left_hip.x + right_shoulder.x + right_hip.x) / 4;
    double center_y = (left_shoulder.y + left_hip.y + right_shoulder.y + right_hip.y) / 4;
    double torso_height = ((left_hip.y - left_shoulder.y) + (right_hip.y - right_shoulder.y)) / 2;
    for (string body_part : pose.keys()) {
      Point& point = pose[body_part];
      point.x = (point.x - center_x) / torso_height;
      point.y = (point.y - center_y) / torso_height;
    }
  }
}

Polys Scorer::fit(vector<Pose>& poses) {
  Polys coeffs;
  for (string body_part : PoseEstimator::body_parts) {
    vector<int> idxs;
    for (int i = 0; i < poses.size(); i++) {
      if (poses[i].contains(body_part)) {
        idxs.push_back(i);
      }
    }
    if (idxs.size() <= poly_degree) {
      continue;
    }
    Eigen::MatrixXd a = Eigen::MatrixXd(idxs.size(), poly_degree + 1);
    Eigen::VectorXd b_x = Eigen::VectorXd(idxs.size());
    Eigen::VectorXd b_y = Eigen::VectorXd(idxs.size());
    for (int i = 0; i < idxs.size(); i++) {
      int idx = idxs[i];
      for (int j = 0; j <= poly_degree; j++) {
        int d = poly_degree - j;
        a(i, j) = std::pow(idx, d);
      }
      Point point = poses[idx][body_part];
      b_x(i) = point.x;
      b_y(i) = point.y;
    }
    Eigen::MatrixXd p = a * (a.transpose() * a).inverse() * a.transpose();
    Eigen::VectorXd p_x = p * b_x;
    Eigen::VectorXd p_y = p * b_y;
    Eigen::VectorXd eigen_x_coeffs = a.colPivHouseholderQr().solve(p_x);
    Eigen::VectorXd eigen_y_coeffs = a.colPivHouseholderQr().solve(p_y);
    Coeffs x_coeffs = Coeffs(eigen_x_coeffs.data(), eigen_x_coeffs.data() + eigen_x_coeffs.size());
    Coeffs y_coeffs = Coeffs(eigen_y_coeffs.data(), eigen_y_coeffs.data() + eigen_y_coeffs.size());
    coeffs[body_part] = {x_coeffs, y_coeffs};
  }
  return coeffs;
}

double Scorer::weighted_mse(Polys player_polys, Polys avatar_polys, double t_start, double t_end, double t_offset) {
  if (t_start >= t_end) {
    ERROR("end time must be greater than start time");
  }
  double numerator = 0;
  double denominator = 0;
  for (string body_part : PoseEstimator::body_parts) {
    if (player_polys.contains(body_part) && avatar_polys.contains(body_part)) {
      Coeffs player_x_coeffs = player_polys[body_part].first;
      Coeffs player_y_coeffs = player_polys[body_part].second;
      Coeffs avatar_x_coeffs = avatar_polys[body_part].first;
      Coeffs avatar_y_coeffs = avatar_polys[body_part].second;
      Coeffs avatar_dx_coeffs = differentiate(avatar_x_coeffs);
      Coeffs avatar_dy_coeffs = differentiate(avatar_y_coeffs);
      for (double t = t_start; t < t_end; t += resolution) {
        double x_diff = evaluate(avatar_x_coeffs, t) - evaluate(player_x_coeffs, t - t_offset);
        double y_diff = evaluate(avatar_y_coeffs, t) - evaluate(player_y_coeffs, t - t_offset);
        double dx = evaluate(avatar_dx_coeffs, t);
        double dy = evaluate(avatar_dy_coeffs, t);
        double wx = dx * dx;
        double wy = dy * dy;
        numerator += wx * x_diff * x_diff + wy * y_diff * y_diff;
        denominator += wx + wy;
      }
    }
  }
  if (denominator == 0) {
    return DBL_MAX;
  }
  double error = numerator / denominator;
  return error;
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

Coeffs Scorer::differentiate(Coeffs coeffs) {
  Coeffs deriv_coeffs;
  int degree = coeffs.size() - 1;
  for (int i = 0; i < coeffs.size() - 1; i++) {
    int d = degree - i;
    deriv_coeffs.push_back(coeffs[i] * d);
  }
  return deriv_coeffs;
}
