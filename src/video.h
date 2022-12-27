#ifndef VIDEO_H_
#define VIDEO_H_

#include "defs.h"
#include "pose.h"

class VideoLoader {
  public:
  void save(string name);
};

class Video {
  public:
  Video(string name);
  void play();
  bool finished();
  cv::Mat currFrame();
  cv::Mat currFooterFrame();
  Pose getPose();
  int length();
  int fps();
  double currTime();
  double totalTime();

  private:
  cv::VideoCapture video;
  cv::VideoCapture footer;
  TimePoint start_time;
  vector<Pose> poses;
  int currIndex();
};

#endif
