#include "video.h"
#include "pose.h"
#include "canvas.h"

void VideoLoader::save(string name) {
  string video_filename = name + ".mp4";
  string pose_filename = name + ".txt";
  cv::VideoCapture capture("videos/" + video_filename);
  if (!capture.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  int curr_frame = 0;
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  capture.read(frame);
  if (frame.empty()) {
    ERROR("empty frame from \"" + video_filename + "\"");
  }
  ofstream pose_file("data/" + pose_filename);
  while(!frame.empty()) {
    Pose pose = pose_estimator.getPose(frame, true);
    if (pose.empty()) {
      pose_file << "empty" << endl;
    } else {
      for (Landmark landmark : pose.landmarks) {
        pose_file << landmark.x << " " << landmark.y << " " << landmark.visibility << endl;
      }
      canvas.renderPose(frame, pose, 255, 0, 255);
    }
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
    curr_frame++;
    cout << "loading: " << std::setprecision(2) << std::fixed << (double) curr_frame / num_frames * 100 << "%" << endl;
    capture.read(frame);
  }
}

Video::Video(string name) {
  string video_filename = name + ".mp4";
  string pose_filename = name + ".txt";
  capture = cv::VideoCapture("videos/" + video_filename);
  if (!capture.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  int num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
  poses.reserve(num_frames);
  ifstream pose_file("data/" + pose_filename);
  if (!pose_file.good()) {
    ERROR("unable to open file \"" + pose_filename + "\"");
  }
  for (int i = 0; i < num_frames; i++) {
    Pose pose;
    while (!pose_file.eof() && std::isspace(pose_file.peek())) {
      pose_file.get();
    }
    if (pose_file.peek() == 'e') {
      string temp;
      pose_file >> temp;
    } else {
      for (string body_part : PoseEstimator::body_parts) {
        double x, y, visibility;
        pose_file >> x >> y >> visibility;
        pose.addLandmark(Landmark(body_part, x, y, visibility));
      }
    }
    poses.push_back(pose);
  }
}

void Video::play() {
  start_time = std::chrono::steady_clock::now();
}

bool Video::finished() {
  return currIndex() >= length();
}

cv::Mat Video::getFrame() {
  for (int i = capture.get(cv::CAP_PROP_POS_FRAMES); i < currIndex() - 1; i++) {
    capture.grab();
  }
  cv::Mat frame;
  capture.read(frame);
  return frame;
}

Pose Video::getPose() {
  return poses[currIndex()];
}

int Video::width() {
  return capture.get(cv::CAP_PROP_FRAME_WIDTH);
}

int Video::height() {
  return capture.get(cv::CAP_PROP_FRAME_HEIGHT);
}

int Video::length() {
  return capture.get(cv::CAP_PROP_FRAME_COUNT);
}

int Video::fps() {
  return capture.get(cv::CAP_PROP_FPS) + 0.5;
}

double Video::currTime() {
  TimePoint curr_time = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - start_time).count() * 1e-9;
}

double Video::totalTime() {
  return (double) length() / fps();
}

int Video::currIndex() {
  return fps() * currTime();
}
