#ifndef DEFS_H_
#define DEFS_H_

#include <iostream>
#include <string>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class Game;
class PoseEstimator;
class Pose;
class Landmark;
class Canvas;
class Video;
class Audio;

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::ifstream;
using std::ofstream;

#define ERROR(message) \
  std::cerr << "\033[1;31mERROR\033[0m (" << __FILE__ << ":" << __LINE__ << "): " << message << endl; \
  exit(EXIT_FAILURE);

#endif
