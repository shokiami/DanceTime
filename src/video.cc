#include "video.h"
#include "pose.h"
#include "canvas.h"
#include <fstream>

void VideoLoader::save(string name) {
  string video_filename = name + ".mp4";
  string csv_filename = name + ".csv";
  cv::VideoCapture capture("videos/" + video_filename);
  if (!capture.isOpened()) {
    ERROR("Unable to open file \"" + video_filename + "\".");
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  int current_frame = 0;
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  capture.read(frame);
  if (frame.empty()) {
    ERROR("Empty frame from \"" + video_filename + "\".");
  }
  ofstream csv_file ("data/" + csv_filename);
  while(!frame.empty()) {
    Pose pose = pose_estimator.getPose(frame, true);
    canvas.renderPose(frame, pose);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
    if (pose.isEmpty()) {
      csv_file << "empty" << endl;
    } else {
      for (string body_part : PoseEstimator::body_parts) {
        Landmark landmark = pose.getLandmark(body_part);
        csv_file << landmark.x << " " << landmark.y << " " << landmark.z << " " << landmark.visibility << endl;
      }
    }
    current_frame++;
    cout << '\r' << std::setprecision (2) << std::fixed;
    cout << "Loading: " << (double) current_frame / num_frames * 100 << "%" << std::flush;
    capture.read(frame);
  }
  cout << endl;
}

Video::Video(string name) : name(name) {
  string video_filename = name + ".mp4";
  string csv_filename = name + ".csv";
  capture = cv::VideoCapture("videos/" + video_filename);
  if (!capture.isOpened()) {
    ERROR("Unable to open file \"" + video_filename + "\".");
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  poses.reserve(num_frames);
  ifstream csv_file("data/" + csv_filename);
  if (!csv_file.good()) {
    ERROR("Unable to open file \"" + csv_filename + "\".");
  }
  for (int i = 0; i < num_frames; i++) {
    Pose pose;
    while (!csv_file.eof() && std::isspace(csv_file.peek())) {
      csv_file.get();
    }
    if (csv_file.peek() == 'e') {
      string temp;
      csv_file >> temp;
    } else {
      for (string body_part : PoseEstimator::body_parts) {
        double x, y, z, visibility;
        csv_file >> x >> y >> z >> visibility;
        pose.addLandmark(Landmark(body_part, x, y, z, visibility));
      }
    }
    poses.push_back(pose);
  }
}

void Video::play() {
  start_time = std::chrono::steady_clock::now();
}

bool Video::isFinished() {
  return getIndex() >= length();
}

cv::Mat Video::getFrame() {
  for (int i = capture.get(cv::CAP_PROP_POS_FRAMES); i < getIndex() - 1; i++) {
    capture.grab();
  }
  cv::Mat frame;
  capture.read(frame);
  return frame;
}

Pose Video::getPose() {
  return poses[getIndex()];
}

string Video::getName() {
  return name;
}

int Video::getWidth() {
  return capture.get(cv::CAP_PROP_FRAME_WIDTH);
}

int Video::getHeight() {
  return capture.get(cv::CAP_PROP_FRAME_HEIGHT);
}

int Video::length() {
  return capture.get(cv::CAP_PROP_FRAME_COUNT);
}

int Video::getFps() {
  return capture.get(cv::CAP_PROP_FPS) + 0.5;
}

double Video::getTime() {
  time_point current_time = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - start_time).count() * 1e-9;
}

double Video::getTotalTime() {
  return (double) length() / getFps();
}

int Video::getIndex() {
  return getFps() * getTime();
}
