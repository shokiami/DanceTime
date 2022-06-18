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
  Game();
  bool finished();
  void update();
  void render();

  private:
  cv::VideoCapture capture;
  PoseEstimator pose_estimator;
  Canvas canvas;
  Video video;
  Audio audio;
  Pose camera_pose;
  Pose video_pose;
  cv::Mat camera_frame;
  cv::Mat video_frame;
  Scorer scorer;
  double score;
  time_point prev_score_time;
  time_point prev_fps_time;
  int fps;
  char keyCode;
};

#endif
