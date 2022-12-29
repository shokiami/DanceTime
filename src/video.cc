#include "video.h"
#include "canvas.h"

void VideoLoader::save(string name) {
  string video_filename = name + ".mp4";
  string pose_filename = name + ".txt";
  cv::VideoCapture video = cv::VideoCapture("videos/" + video_filename);
  if (!video.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  int num_frames = video.get(cv::CAP_PROP_FRAME_COUNT);
  int curr_frame = 0;
  cv::Mat frame;
  PoseEstimator pose_estimator;
  Canvas canvas;
  video.read(frame);
  if (frame.empty()) {
    ERROR("empty frame from \"" + video_filename + "\"");
  }
  std::ofstream pose_file = std::ofstream("data/" + pose_filename);
  cout << std::setprecision(2) << std::fixed;
  while(!frame.empty()) {
    Pose pose = pose_estimator.getPose(frame, true);
    for (string body_part : pose.keys()) {
      Point point = pose[body_part];
      pose_file << body_part << " " << point.x << " " << point.y << " ";
    }
    pose_file << endl;
    canvas.render(frame, pose, 255, 0, 255);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);
    curr_frame++;
    cout << "loading: " << 100.0 * curr_frame / num_frames << "%" << endl;
    video.read(frame);
  }
}

Video::Video(string name) {
  string video_filename = name + ".mp4";
  string footer_filename = name + "_footer.mp4";
  string pose_filename = name + ".txt";
  video = cv::VideoCapture("videos/" + video_filename);
  if (!video.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  footer = cv::VideoCapture("videos/" + footer_filename);
  if (!footer.isOpened()) {
    ERROR("unable to open file \"" + footer_filename + "\"");
  }
  if (footer.get(cv::CAP_PROP_FRAME_COUNT) != video.get(cv::CAP_PROP_FRAME_COUNT)) {
    ERROR("length of video and footer do not match")
  }
  if (footer.get(cv::CAP_PROP_FPS) != video.get(cv::CAP_PROP_FPS)) {
    ERROR("fps of video and footer do not match")
  }
  std::ifstream pose_file = std::ifstream("data/" + pose_filename);
  if (!pose_file.good()) {
    ERROR("unable to open file \"" + pose_filename + "\"");
  }
  string line;
  while (std::getline(pose_file, line)) {
    std::istringstream string_stream = std::istringstream(line);
    Pose pose;
    string body_part;
    double x;
    double y;
    while (string_stream >> body_part >> x >> y) {
      pose[body_part] = Point(x, y);
    }
    poses.push_back(pose);
  }
  if (poses.size() != video.get(cv::CAP_PROP_FRAME_COUNT)) {
    ERROR("length of video and text file do not match");
  }
}

void Video::play() {
  start_time = std::chrono::steady_clock::now();
}

bool Video::finished() {
  return currIndex() >= length();
}

cv::Mat Video::currFrame() {
  for (int i = video.get(cv::CAP_PROP_POS_FRAMES); i < currIndex() - 1; i++) {
    video.grab();
  }
  cv::Mat frame;
  video.read(frame);
  return frame;
}

cv::Mat Video::currFooterFrame() {
  for (int i = footer.get(cv::CAP_PROP_POS_FRAMES); i < currIndex() - 1; i++) {
    footer.grab();
  }
  cv::Mat frame;
  footer.read(frame);
  return frame;
}

Pose Video::getPose() {
  return poses[currIndex()];
}

int Video::length() {
  return video.get(cv::CAP_PROP_FRAME_COUNT);
}

int Video::fps() {
  return video.get(cv::CAP_PROP_FPS) + 0.5;
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
