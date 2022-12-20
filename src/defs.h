#ifndef DEFS_H_
#define DEFS_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class Game;
class PoseEstimator;
class Pose;
class Landmark;
class Canvas;
class Video;
class Audio;
class Scorer;

using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;
using std::pair;
using std::ifstream;
using std::ofstream;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

#define ERROR(message) \
  cerr << "\033[1;31mERROR\033[0m (" << __FILE__ << ":" << __LINE__ << "): " << message << endl; \
  exit(EXIT_FAILURE);

#endif
