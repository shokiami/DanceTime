#ifndef VIDEO_H_
#define VIDEO_H_

#include "defs.h"

class VideoLoader {
  public:
  void save(string name);
};

class Video {
  public:
  Video(string name);
  void play();
  bool finished();
  cv::Mat getFrame();
  Pose getPose();
  int width();
  int height();
  int length();
  int fps();
  double currTime();
  double totalTime();

  private:
  cv::VideoCapture capture;
  TimePoint start_time;
  vector<Pose> poses;
  int currIndex();
};

#endif
