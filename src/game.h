#ifndef GAME_H_
#define GAME_H_

#include "defs.h"
#include "pose.h"
#include "canvas.h"
#include "video.h"
#include "audio.h"
#include "scorer.h"

class Game {
  public:
  Game(string name);
  bool finished();
  void update();
  void render();

  private:
  cv::VideoCapture capture;
  PoseEstimator pose_estimator;
  Canvas canvas;
  Video video;
  Audio audio;
  Pose player_pose;
  Pose avatar_pose;
  vector<Pose> player_history;
  vector<Pose> avatar_history;
  cv::Mat camera_frame;
  cv::Mat video_frame;
  cv::Mat footer_frame;
  Scorer scorer;
  double score;
  TimePoint prev_score_time;
  TimePoint prev_fps_time;
  double fps;
  bool debug;
  char key_code;
};

#endif
