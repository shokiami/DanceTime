#include "game.h"

Game::Game() : capture(0), video("blackpink"), audio("blackpink") {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
}

bool Game::finished() {
  return keyCode == 'q' || video.finished();
}

void Game::update() {
  // get camera pose
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("Empty frame from camera.");
  }
  cv::flip(camera_frame, camera_frame, 1);
  camera_pose = pose_estimator.getPose(camera_frame);
  // get video pose
  video_frame = video.getFrame();
  if (video_frame.empty()) {
    ERROR("Empty frame from video.");
  }
  video_pose = video.getPose();
  // update audio
  audio.update();
  // update score
  scorer.addPoses(camera_pose, video_pose);
  time_point curr_time = std::chrono::steady_clock::now();
  double elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count() * 1e-9;
  if (elapsed_time > 1) {
    score = scorer.getScore();
    scorer.reset();
    prev_score_time = curr_time;
  }
  // update fps
  elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count() * 1e-9;
  prev_fps_time = curr_time;
  fps = 1 / elapsed_time;
}

void Game::render() {
  // render camera pose
  if (!camera_pose.empty()) {
    canvas.renderPose(camera_frame, camera_pose, cv::Scalar(255, 125, 75));
  }
  int camera_height = camera_frame.size[0];
  int camera_width = camera_frame.size[1];
  int target_width = video.width() * camera_height / video.height();
  camera_frame = camera_frame(cv::Range(0, camera_height),
    cv::Range(camera_width / 2 - target_width / 2, camera_width / 2 + target_width / 2));
  // render video pose
  resize(video_frame, video_frame, cv::Size(target_width, camera_height));
  if (!video_pose.empty()) {
    canvas.renderPose(video_frame, video_pose, cv::Scalar(255, 0, 255));
  }
  // concatenate and show frames
  cv::Mat frame;
  cv::hconcat(video_frame, camera_frame, frame);
  cv::imshow("DanceTime", frame);
  keyCode = cv::waitKey(1) & 0xFF;
  // print fps and score
  cout << '\r' << "FPS: " << fps << " Score: " << score << std::flush;
}
