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
    ERROR("number of poses does not match");
  }
  // standardize data
  int player_nonempty = 0;
  for (Pose& player_pose : player_poses) {
    if (!player_pose.empty()) {
      player_pose.standardize();
      player_nonempty++;
    }
  }
  if (player_nonempty <= poly_degree) {
    return 0;
  }
  int avatar_nonempty = 0;
  for (Pose& avatar_pose : avatar_poses) {
    if (!avatar_pose.empty()) {
      avatar_pose.standardize();
      avatar_nonempty++;
    }
  }
  if (avatar_nonempty <= poly_degree) {
    return 0;
  }

  // compute best mse and time offset
  Polys player_polys = fit(player_poses);
  Polys avatar_polys = fit(avatar_poses);
  double t_elapsed = player_poses.size();
  double best_error = DBL_MAX;
  double best_offset = 0;
  for (double t_offset = -t_elapsed; t_offset <= t_elapsed; t_offset += resolution) {
    double error = mse(player_polys, avatar_polys, 0, t_elapsed, t_offset);
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

vector<string> Scorer::body_parts = {"left wrist", "right wrist"};

Polys Scorer::fit(vector<Pose> poses) {
vector<int> idxs;
  for (int i = 0; i < poses.size(); i++) {
    if (!poses[i].empty()) {
      idxs.push_back(i);
    }
  }
  int n = idxs.size();
  if (n <= poly_degree) {
    ERROR("not enough data points for regression");
  }
  Eigen::MatrixXd a = Eigen::MatrixXd(n, poly_degree + 1);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= poly_degree; j++) {
      int d = poly_degree - j;
      a(i, j) = std::pow(idxs[i], d);
    }
  }
  Eigen::MatrixXd p = a * (a.transpose() * a).inverse() * a.transpose();
  Polys coeffs;
  for (string body_part : body_parts) {
    Eigen::VectorXd b_x = Eigen::VectorXd(n);
    Eigen::VectorXd b_y = Eigen::VectorXd(n);
    for (int i = 0; i < n; i++) {
      Landmark landmark = poses[idxs[i]].getLandmark(body_part);
      b_x(i) = landmark.x;
      b_y(i) = landmark.y;
    }
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

double Scorer::mse(Polys player_polys, Polys avatar_polys, double t_start, double t_end, double t_offset) {
  if (t_start >= t_end) {
    ERROR("end time must be greater than start time");
  }
  double numerator = 0;
  double denominator = 0;
  for (string body_part : body_parts) {
    Coeffs player_x_coeffs = player_polys[body_part].first;
    Coeffs player_y_coeffs = player_polys[body_part].second;
    Coeffs avatar_x_coeffs = avatar_polys[body_part].first;
    Coeffs avatar_y_coeffs = avatar_polys[body_part].second;
    for (double t = t_start; t < t_end; t += resolution) {
      double x_diff = evaluate(avatar_x_coeffs, t) - evaluate(player_x_coeffs, t - t_offset);
      double y_diff = evaluate(avatar_y_coeffs, t) - evaluate(player_y_coeffs, t - t_offset);
      numerator += x_diff * x_diff + y_diff * y_diff;
      denominator += 1;
    }
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
