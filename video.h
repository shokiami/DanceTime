#ifndef VIDEO_H_
#define VIDEO_H_

#include "defs.h"

using time_point = std::chrono::time_point<std::chrono::steady_clock>;

class VideoLoader {
  public:
  void saveVideo(std::string name);
};

class Video {
  public:
  Video(std::string name);
  void play();
  bool finished();
  double getTime();
  cv::Mat getFrame();
  Pose getPose();
  std::string getName();
  int length();

  private:
  std::string name;
  time_point start_time;
  int num_frames;
  int current_frame;
  double fps;
  cv::VideoCapture capture;
  std::vector<Pose> poses;
  int getIndex();
};

#endif
