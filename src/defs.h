#ifndef DEFS_H_
#define DEFS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;
using std::pair;
using std::unordered_map;

typedef cv::Point2d Point;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

#define ERROR(message) \
  cerr << "\033[1;31mERROR\033[0m (" << __FILE__ << ":" << __LINE__ << "): " << message << endl; \
  exit(EXIT_FAILURE);

#endif
