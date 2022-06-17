#include "game.h"

Game::Game() : capture(0), video("blackpink"), audio("blackpink") {
  audio.play();
  video.play();
  previous_time = std::chrono::steady_clock::now();
}

bool Game::isFinished() {
  return keyCode == 'q' || video.isFinished();
}

void Game::update() {
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("Empty frame from camera.");
  }
  cv::flip(camera_frame, camera_frame, 1);
  camera_pose = pose_estimator.getPose(camera_frame);
  
  video_frame = video.getFrame();
  if (video_frame.empty()) {
    ERROR("Empty frame from video.");
  }
  video_pose = video.getPose();
  
  audio.update();

  time_point current_time = std::chrono::steady_clock::now();
  double elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time).count() * 1e-9;
  previous_time = current_time;
  fps = 1 / elapsed_time;
}

void Game::render() {
  canvas.renderPose(camera_frame, camera_pose, cv::Scalar(255, 125, 75));
  int camera_height = camera_frame.size[0];
  int camera_width = camera_frame.size[1];
  int target_width = video.getWidth() * camera_height / video.getHeight();
  camera_frame = camera_frame(cv::Range(0, camera_height),
    cv::Range(camera_width / 2 - target_width / 2, camera_width / 2 + target_width / 2));
  
  resize(video_frame, video_frame, cv::Size(target_width, camera_height));
  canvas.renderPose(video_frame, video_pose, cv::Scalar(255, 0, 255));

  cv::Mat frame;
  cv::hconcat(video_frame, camera_frame, frame);
  cv::imshow("DanceTime", frame);
  keyCode = cv::waitKey(1) & 0xFF;
  
  cout << '\r' << "FPS: " << fps << std::flush;
}
