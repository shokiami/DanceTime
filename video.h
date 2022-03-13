#ifndef VIDEO_H_
#define VIDEO_H_

#include "defs.h"

using time_point = std::chrono::time_point<std::chrono::steady_clock>;

class VideoLoader {
  public:
  void saveVideo(string name);
};

class Video {
  public:
  Video(string name);
  void play();
  bool finished();
  double getTime();
  cv::Mat getFrame();
  Pose getPose();
  string getName();
  int getWidth();
  int getHeight();
  int length();
  double getFPS();

  private:
  string name;
  time_point start_time;
  int current_frame;
  cv::VideoCapture capture;
  vector<Pose> poses;
  int getIndex();
};

#endif
