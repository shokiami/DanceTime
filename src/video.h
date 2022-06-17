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
  bool isFinished();
  cv::Mat getFrame();
  Pose getPose();
  string getName();
  int getWidth();
  int getHeight();
  int length();
  int getFps();
  double getTime();
  double getTotalTime();

  private:
  string name;
  cv::VideoCapture capture;
  time_point start_time;
  vector<Pose> poses;
  int getIndex();
};

#endif
