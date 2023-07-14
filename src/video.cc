#include "video.h"
#include "canvas.h"

void VideoSaver::save(string name) {
  // open video file
  string video_filename = name + ".mp4";
  cv::VideoCapture video = cv::VideoCapture("videos/" + video_filename);
  if (!video.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }

  // open text file
  string text_filename = name + ".txt";
  std::ofstream text_file = std::ofstream("data/" + text_filename);
  if (!text_file) {
    ERROR("unable to open file \"" + text_filename + "\"")
  }

  // save video
  PoseEstimator pose_estimator;
  Canvas canvas;
  cv::Mat frame;
  video.read(frame);
  if (frame.empty()) {
    ERROR("empty frame from \"" + video_filename + "\"");
  }
  int curr_frame = 0;
  cout << std::setprecision(2) << std::fixed;
  while(!frame.empty()) {
    // run pose estimation in synchronous mode
    Pose pose = pose_estimator.getPose1(frame, true);

    // write pose to text file
    for (string body_part : pose.keys()) {
      Point point = pose[body_part];
      text_file << body_part << " " << point.x << " " << point.y << " ";
    }
    text_file << endl;

    // render pose
    canvas.render(frame, pose, 255, 0, 255);
    cv::imshow("DanceTime", frame);
    cv::waitKey(1);

    // print progress
    curr_frame++;
    cout << "loading: " << 1e2 * curr_frame / video.get(cv::CAP_PROP_FRAME_COUNT) << "%" << endl;

    // read next frame
    video.read(frame);
  }
}

Video::Video(string name) {
  // open video file
  string video_filename = name + ".mp4";
  video = cv::VideoCapture("videos/" + video_filename);
  if (!video.isOpened()) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }

  // open footer file 
  string footer_filename = name + "_footer.mp4";
  footer = cv::VideoCapture("videos/" + footer_filename);
  if (!footer.isOpened()) {
    ERROR("unable to open file \"" + footer_filename + "\"");
  }

  // make sure video and footer file are compatable
  if (footer.get(cv::CAP_PROP_FRAME_COUNT) != video.get(cv::CAP_PROP_FRAME_COUNT)) {
    ERROR("length of video and footer do not match")
  }
  if (footer.get(cv::CAP_PROP_FPS) != video.get(cv::CAP_PROP_FPS)) {
    ERROR("fps of video and footer do not match")
  }

  // open text file 
  string text_filename = name + ".txt";
  std::ifstream text_file = std::ifstream("data/" + text_filename);
  if (!text_file) {
    ERROR("unable to open file \"" + text_filename + "\"");
  }

  // load poses from text file
  string line;
  while (std::getline(text_file, line)) {
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
  // skip to current frame
  for (int i = video.get(cv::CAP_PROP_POS_FRAMES); i < currIndex() - 1; i++) {
    video.grab();
  }

  // read current frame
  cv::Mat frame;
  video.read(frame);
  return frame;
}

cv::Mat Video::currFooterFrame() {
  // skip to current frame
  for (int i = footer.get(cv::CAP_PROP_POS_FRAMES); i < currIndex() - 1; i++) {
    footer.grab();
  }

  // read current frame
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
  return 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - start_time).count();
}

double Video::totalTime() {
  return (double) length() / fps();
}

int Video::currIndex() {
  return fps() * currTime();
}
