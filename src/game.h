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
  pair<double, double> results();

  private:
  cv::VideoCapture capture;
  PoseEstimator pose_estimator;
  Canvas canvas;
  Video video;
  Audio audio;
  cv::Mat player1_frame;
  cv::Mat player2_frame;
  cv::Mat video_frame;
  cv::Mat footer_frame;
  Pose player1_pose;
  Pose player2_pose;
  Pose avatar_pose;
  vector<Pose> player1_history;
  vector<Pose> player2_history;
  vector<Pose> avatar_history;
  Scorer scorer;
  int player1_score;
  int player2_score;
  int player1_total_score;
  int player2_total_score;
  int seconds_passed;
  TimePoint prev_score_time;
  TimePoint prev_fps_time;
  double fps;
  bool debug;
  char key_code;
};

#endif
