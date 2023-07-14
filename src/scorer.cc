#include "scorer.h"
#include <eigen3/Eigen/Dense>
#include <float.h>

// body parts used to score
const vector<string> Scorer::body_parts = {"left_wrist", "right_wrist"};

double Scorer::score(vector<Pose> player_poses, vector<Pose> avatar_poses) {
  if (player_poses.size() != avatar_poses.size()) {
    ERROR("number of poses does not match");
  }

  // handle not enough poses case
  if (player_poses.size() <= poly_degree || player_poses.size() <= 2.0 * padding) {
    return 0.0;
  }

  // remove outliers
  remove_outliers(player_poses);
  remove_outliers(avatar_poses);

  // standardize data
  standardize(player_poses);
  standardize(avatar_poses);

  // compute polynomial approximations to data using regression
  Polys player_polys = fit(player_poses);
  Polys avatar_polys = fit(avatar_poses);

  // handle out of frame cases
  if (avatar_polys.empty()) {
    return 1.0;
  }
  if (player_polys.empty()) {
    return 0.0;
  }

  // scan for offset which yields smallest max error
  double elapsed_time = player_poses.size();
  double best_error = DBL_MAX;
  double best_offset = 0.0;
  for (double offset = -max_offset; offset <= max_offset; offset += resolution) {
    double error = max_error(player_polys, avatar_polys, padding, elapsed_time - padding, offset);
    if (error < best_error) {
      best_error = error;
      best_offset = offset;
    }
  }

  // calculate score from error and offset
  return error_to_score(best_error + offset_cost * std::abs(best_offset));
}

bool Scorer::inframe(Pose& pose) {
  return pose.contains("left_shoulder") && pose.contains("left_hip")
    && pose.contains("right_shoulder") && pose.contains("right_hip");
}

void Scorer::standardize(Pose& pose) {
  Point left_shoulder = pose["left_shoulder"];
  Point left_hip = pose["left_hip"];
  Point right_shoulder = pose["right_shoulder"];
  Point right_hip = pose["right_hip"];

  // get torso center
  Point torso_center = (left_shoulder + left_hip + right_shoulder + right_hip) / 4.0;

  // get torso length
  double torso_left_x_diff = left_shoulder.x - left_hip.x;
  double torso_left_y_diff = left_shoulder.y - left_hip.y;
  double torso_right_x_diff = right_shoulder.x - right_hip.x;
  double torso_right_y_diff = right_shoulder.y - right_hip.y;
  double torso_left_length = std::sqrt(torso_left_x_diff * torso_left_x_diff + torso_left_y_diff * torso_left_y_diff);
  double torso_right_length = std::sqrt(torso_right_x_diff * torso_right_x_diff + torso_right_y_diff * torso_right_y_diff);
  double torso_length = (torso_left_length + torso_right_length) / 2.0;

  // standardize
  for (string body_part : pose.keys()) {
    Point& point = pose[body_part];
    point = (point - torso_center) / torso_length;
  }
}

void Scorer::remove_outliers(vector<Pose>& poses) {
  vector<Pose> new_poses;
  for (int i = 0; i < poses.size(); i++) {
    // skip processing for first and last pose
    if (i == 0 || i == poses.size() - 1) {
      new_poses.push_back(poses[i]);
      continue;
    }

    Pose new_pose;
    for (string body_part : poses[i].keys()) {
      // skip processing if neighboring poses do not contain body part
      Point curr_point = poses[i][body_part];
      new_pose[body_part] = curr_point;
      if (!poses[i - 1].contains(body_part) || !poses[i + 1].contains(body_part)) {
        continue;
      }

      // get distances between previous, current, and next points for body part
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

      // if current is an outlier, set it to average of previous and next
      if (ops_sq_dist < adj1_sq_dist || ops_sq_dist < adj2_sq_dist) {
        new_pose[body_part] = (prev_point + next_point) / 2.0;
      }
    }

    new_poses.push_back(new_pose);
  }

  poses = new_poses;
}

void Scorer::standardize(vector<Pose>& poses) {
  for (Pose& pose : poses) {
    // standardize pose if in frame, otherwise set to empty pose
    if (inframe(pose)) {
      standardize(pose);
    } else {
      pose = Pose();
    }
  }
}

Polys Scorer::fit(vector<Pose>& poses) {
  Polys polys;
  for (string body_part : body_parts) {
    // get indices of poses that contain body part
    vector<int> idxs;
    for (int i = 0; i < poses.size(); i++) {
      if (poses[i].contains(body_part)) {
        idxs.push_back(i);
      }
    }

    // if not enough indices, skip body part
    if (idxs.size() <= poly_degree) {
      continue;
    }

    // prepare a * x = b_x and a * y = b_y
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

    // get projection matrix that projects onto col(a)
    Eigen::MatrixXd p = a * (a.transpose() * a).inverse() * a.transpose();

    // project b_x and b_y
    Eigen::VectorXd p_x = p * b_x;
    Eigen::VectorXd p_y = p * b_y;

    // solve for x and y coefficients
    Eigen::VectorXd eigen_x_coeffs = a.colPivHouseholderQr().solve(p_x);
    Eigen::VectorXd eigen_y_coeffs = a.colPivHouseholderQr().solve(p_y);

    // convert eigen vectors to std vectors
    Coeffs x_coeffs = Coeffs(eigen_x_coeffs.data(), eigen_x_coeffs.data() + eigen_x_coeffs.size());
    Coeffs y_coeffs = Coeffs(eigen_y_coeffs.data(), eigen_y_coeffs.data() + eigen_y_coeffs.size());
    polys[body_part] = {x_coeffs, y_coeffs};
  }

  return polys;
}

double Scorer::max_error(Polys player_polys, Polys avatar_polys, double start_time, double end_time, double offset) {
  if (start_time >= end_time) {
    ERROR("end time must be greater than start time");
  }

  double max_error = -1.0;
  for (string body_part : body_parts) {
    if (player_polys.contains(body_part) && avatar_polys.contains(body_part)) {
      // get all coefficients corresponding to body part
      Coeffs player_x_coeffs = player_polys[body_part].first;
      Coeffs player_y_coeffs = player_polys[body_part].second;
      Coeffs avatar_x_coeffs = avatar_polys[body_part].first;
      Coeffs avatar_y_coeffs = avatar_polys[body_part].second;

      // approximate max squared error between p(t - offset) and a(t)
      for (double t = start_time; t < end_time; t += resolution) {
        double x_diff = evaluate(player_x_coeffs, t - offset) - evaluate(avatar_x_coeffs, t);
        double y_diff = evaluate(player_y_coeffs, t - offset) - evaluate(avatar_y_coeffs, t);
        double error = x_diff * x_diff + y_diff * y_diff;
        if (error > max_error) {
          max_error = error;
        }
      }
    }
  }

  // if there was no shared body parts between player polys and avatar polys, return infinity
  if (max_error == -1.0) {
    return DBL_MAX;
  }

  return max_error;
}

double Scorer::evaluate(Coeffs coeffs, double t) {
  int degree = coeffs.size() - 1;

  // evaluate polynomial with given coefficients at t
  double result = 0.0;
  for (int i = 0; i < coeffs.size(); i++) {
    int d = degree - i;
    result += coeffs[i] * std::pow(t, d);
  }

  return result;
}

double Scorer::error_to_score(double error) {
  return 1.0 / (1.0 + std::pow(intercept / (1.0 - intercept), error / midpoint - 1.0));
}
