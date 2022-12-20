#ifndef SCORER_H_
#define SCORER_H_

#include "defs.h"
#include "pose.h"

class Scorer {
  public:
  void reset();
  void addPoses(Pose player_pose, Pose avatar_pose);
  double getScore();

  private:
  vector<Pose> player_poses;
  vector<Pose> avatar_poses;
  void denoise(vector<Pose>& poses);
  double mse(Pose pose1, Pose pose2);
  static constexpr double offset_cost = 0.01;
  static constexpr double sensitivity = 0.001;
};

#endif
