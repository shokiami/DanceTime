#include "video.h"
#include "pose.h"
#include "canvas.h"
#include <fstream>

void VideoLoader::saveVideo(std::string name) {
  cv::VideoCapture capture(name + ".mp4");
  if (!capture.isOpened()) {
    std::cerr << "ERROR: Unable to open file \"" << name << ".mp4 \" in VideoLoader::saveVideo()." << std::endl;
    exit(EXIT_FAILURE);
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  int current_frame = 0;
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  capture.read(frame);
  if (frame.empty()) {
    std::cerr << "ERROR: Empty frame from \"" << name << ".mp4 \" in VideoLoader::saveVideo()." << std::endl;
    exit(EXIT_FAILURE);
  }
  std::ofstream file (name + ".csv");
  while(!frame.empty()) {
    Pose pose = pose_estimator.getPose(frame, true);
    canvas.renderPose(frame, pose);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
    if (pose.isEmpty()) {
      file << "empty" << std::endl;
    } else {
      for (std::string body_part : PoseEstimator::body_parts) {
        Landmark landmark = pose.getLandmark(body_part);
        file << landmark.getPosition().x << " ";
        file << landmark.getPosition().y << " ";
        file << landmark.getPosition().z << " ";
        file << landmark.getVisibility() << " ";
        file << landmark.getPresence() << std::endl;
      }
    }
    current_frame++;
    std::cout << (double) current_frame / num_frames << std::endl;
    capture.read(frame);
  }
}

Video::Video(std::string name) : name(name), capture(name + ".mp4") {
  // capture = cv::VideoCapture(name + ".mp4");
  if (!capture.isOpened()) {
    std::cerr << "ERROR: Unable to open file \"" << name << ".mp4 \" in Video::Video()." << std::endl;
    exit(EXIT_FAILURE);
  }
  num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  fps = capture.get(cv::CAP_PROP_FPS);
  poses.reserve(num_frames);
  std::ifstream file (name + ".csv");
  for (int i = 0; i < num_frames; i++) {
    Pose pose;
    while (!file.eof() && std::isspace(file.peek())) {
      file.get();
    }
    if (file.peek() == 'e') {
      std::string temp;
      file >> temp;
    } else {
      for (std::string body_part : PoseEstimator::body_parts) {
        double x, y, z, visibility, presence;
        file >> x >> y >> z >> visibility >> presence;
        cv::Point3d position(x, y, z);
        pose.addLandmark(Landmark(body_part, position, visibility, presence));
      }
    }
    poses.push_back(pose);
  }
  play();
}

void Video::play() {
  current_frame = 0;
  start_time = std::chrono::steady_clock::now();
}

bool Video::finished() {
  return getIndex() >= num_frames;
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

std::string Video::getName() {
  return name;
}

int Video::length() {
  return num_frames;
}

double Video::getTime() {
  time_point current_time = std::chrono::steady_clock::now();
  return (double) std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - start_time).count() / 1e9;
}

int Video::getIndex() {
  return (int) fps * getTime();
}
