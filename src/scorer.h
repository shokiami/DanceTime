#ifndef SCORER_H_
#define SCORER_H_

#include "defs.h"
#include "pose.h"

typedef vector<double> Coeffs;
typedef Map<string, pair<Coeffs, Coeffs>> Polys;

class Scorer {
  public:
  double score(vector<Pose> player_poses, vector<Pose> avatar_poses);
  bool inframe(Pose& pose);
  void standardize(Pose& pose);

  private:
  void remove_outliers(vector<Pose>& poses);
  void standardize(vector<Pose>& poses);
  Polys fit(vector<Pose>& poses);
  double max_error(Polys player_polys, Polys avatar_polys, double start_time, double end_time, double offset);
  double evaluate(Coeffs coeffs, double t);
  double error_to_score(double error);
  static const vector<string> body_parts;
  static constexpr int poly_degree = 3;
  static constexpr double resolution = 0.1;
  static constexpr double padding = 5;
  static constexpr double max_offset = 5;
  static constexpr double offset_cost = 0.02;
  static constexpr double midpoint = 0.5;
  static constexpr double intercept = 0.99;
};

#endif
