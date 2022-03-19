#ifndef GAME_H_
#define GAME_H_

#include "defs.h"
#include "pose.h"
#include "canvas.h"
#include "video.h"
#include "audio.h"

using time_point = std::chrono::time_point<std::chrono::steady_clock>;

class Game {
  public:
  Game();
  ~Game();
  bool isFinished();
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
  char keyCode;
  time_point previous_time;
  int fps;
};

#endif
