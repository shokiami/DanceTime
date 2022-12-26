#ifndef SCORER_H_
#define SCORER_H_

#include "defs.h"
#include "pose.h"

typedef vector<double> Coeffs;
typedef unordered_map<string, pair<Coeffs, Coeffs>> Polys;

class Scorer {
  public:
  void reset();
  void addPoses(Pose player_pose, Pose avatar_pose);
  double getScore();

  private:
  vector<Pose> player_poses;
  vector<Pose> avatar_poses;
  Polys fit(vector<Pose> poses);
  double getError(Polys player_polys, Polys avatar_polys, double t_start, double t_end, double t_offset);
  double evaluate(Coeffs coeffs, double t);
  Coeffs differentiate(Coeffs coeffs);
  static constexpr double resolution = 1;
  static constexpr double offset_cost = 0.01;
  static constexpr double sensitivity = 0.001;
  static constexpr int poly_degree = 3;
  static vector<string> body_parts;
};

#endif
