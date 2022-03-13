#include "video.h"
#include "pose.h"
#include "canvas.h"
#include <fstream>

void VideoLoader::saveVideo(string name) {
  cv::VideoCapture capture(name + ".mp4");
  if (!capture.isOpened()) {
    cerr << "ERROR: Unable to open file \"" << name << ".mp4 \" in VideoLoader::saveVideo()." << endl;
    exit(EXIT_FAILURE);
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  int current_frame = 0;
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  capture.read(frame);
  if (frame.empty()) {
    cerr << "ERROR: Empty frame from \"" << name << ".mp4 \" in VideoLoader::saveVideo()." << endl;
    exit(EXIT_FAILURE);
  }
  ofstream file (name + ".csv");
  while(!frame.empty()) {
    Pose pose = pose_estimator.getPose(frame, true);
    canvas.renderPose(frame, pose);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
    if (pose.isEmpty()) {
      file << "empty" << endl;
    } else {
      for (string body_part : PoseEstimator::body_parts) {
        Landmark landmark = pose.getLandmark(body_part);
        file << landmark.getPosition().x << " ";
        file << landmark.getPosition().y << " ";
        file << landmark.getPosition().z << " ";
        file << landmark.getVisibility() << " ";
        file << landmark.getPresence() << endl;
      }
    }
    current_frame++;
    cout << (double) current_frame / num_frames << endl;
    capture.read(frame);
  }
}

Video::Video(string name) : name(name), capture(name + ".mp4") {
  if (!capture.isOpened()) {
    cerr << "ERROR: Unable to open file \"" << name << ".mp4 \" in Video::Video()." << endl;
    exit(EXIT_FAILURE);
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  poses.reserve(num_frames);
  ifstream file (name + ".csv");
  cout << "Loading " << name << ".mp4..." << endl;
  for (int i = 0; i < num_frames; i++) {
    Pose pose;
    while (!file.eof() && std::isspace(file.peek())) {
      file.get();
    }
    if (file.peek() == 'e') {
      string temp;
      file >> temp;
    } else {
      for (string body_part : PoseEstimator::body_parts) {
        double x, y, z, visibility, presence;
        file >> x >> y >> z >> visibility >> presence;
        cv::Point3d position(x, y, z);
        pose.addLandmark(Landmark(body_part, position, visibility, presence));
      }
    }
    poses.push_back(pose);
  }
}

void Video::play() {
  current_frame = 0;
  start_time = std::chrono::steady_clock::now();
}

bool Video::finished() {
  return getIndex() >= length();
}

cv::Mat Video::getFrame() {
  while (current_frame < getIndex() - 1) {
    capture.grab();
    current_frame++;
  }
  cv::Mat frame;
  capture.read(frame);
  current_frame++;
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

double Video::getFPS() {
  return capture.get(cv::CAP_PROP_FPS);
}

double Video::getTime() {
  time_point current_time = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - start_time).count() * 1e-9;
}

int Video::getIndex() {
  return getFPS() * getTime();
}
