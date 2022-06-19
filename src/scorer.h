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
  Pose filter(Pose prev, Pose curr, Pose next);
};

#endif
