#ifndef SCORER_H_
#define SCORER_H_

#include "defs.h"
#include "pose.h"

class Scorer {
  public:
  void reset();
  void addPoses(Pose camera_pose, Pose video_pose);
  double getScore();

  private:
  vector<Pose> camera_poses;
  vector<Pose> video_poses;
  Pose filter(Pose prev, Pose curr, Pose next);
};

#endif
