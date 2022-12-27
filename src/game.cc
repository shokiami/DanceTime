#include "game.h"

Game::Game(string name) : capture(0), video(name), audio(name) {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
}

bool Game::finished() {
  return key_code == 'q' || video.finished();
}

void Game::update() {
  // get camera frame
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("empty frame from camera");
  }
  cv::flip(camera_frame, camera_frame, 1);
  // get video frame
  video_frame = video.currFrame();
  if (video_frame.empty()) {
    ERROR("empty frame from video");
  }
  // get footer frame
  footer_frame = video.currFooterFrame();
  if (footer_frame.empty()) {
    ERROR("empty frame from video");
  }
  // get poses
  player_pose = pose_estimator.getPose(camera_frame);
  avatar_pose = video.getPose();
  // update audio
  audio.update();
  // update score
  scorer.add(player_pose, avatar_pose);
  TimePoint curr_time = std::chrono::steady_clock::now();
  double elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count() * 1e-9;
  if (elapsed_time > 1) {
    prev_score_time = curr_time;
    score = scorer.score();
    scorer.reset();
    // print fps and score
    cout << "fps: " << fps << ", score: " << score << endl;
  }
  // update fps
  elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count() * 1e-9;
  prev_fps_time = curr_time;
  fps = 1 / elapsed_time;
}

void Game::render() {
  double video_scalar = camera_frame.rows / ((double) video_frame.cols / footer_frame.cols * footer_frame.rows + video_frame.rows);
  double footer_scalar = video_scalar * video_frame.cols / footer_frame.cols;
  // render player's pose
  // canvas.render(camera_frame, player_pose, 75, 125, 255);
  camera_frame = camera_frame(cv::Range(0, camera_frame.rows),
    cv::Range(0.5 * camera_frame.cols - 0.75 * video_scalar * video_frame.cols,
    0.5 * camera_frame.cols + 0.75 * video_scalar * video_frame.cols));
  // render avatar's pose
  cv::resize(video_frame, video_frame, cv::Size(video_scalar * video_frame.cols, video_scalar * video_frame.rows));
  for (string body_part : avatar_pose.keys()) {
    avatar_pose[body_part] *= video_scalar;
  }
  // canvas.render(video_frame, avatar_pose, 255, 0, 255);
  // resize footer
  int adjustment = camera_frame.rows - (footer_scalar * footer_frame.rows + video_frame.rows) + 0.5;
  cv::resize(footer_frame, footer_frame, cv::Size(footer_scalar * footer_frame.cols, footer_scalar * footer_frame.rows + adjustment));
  // concatenate and display frames
  cv::Mat frame;
  cv::vconcat(video_frame, footer_frame, frame);
  cv::hconcat(frame, camera_frame, frame);
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}
