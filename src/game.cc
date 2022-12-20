#include "game.h"

Game::Game() : capture(0), video("blackpink"), audio("blackpink") {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
}

bool Game::finished() {
  return key_code == 'q' || video.finished();
}

void Game::update() {
  // get player's pose
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("empty frame from camera");
  }
  cv::flip(camera_frame, camera_frame, 1);
  player_pose = pose_estimator.getPose(camera_frame);
  // get avatar's pose
  video_frame = video.getFrame();
  if (video_frame.empty()) {
    ERROR("empty frame from video");
  }
  avatar_pose = video.getPose();
  // update audio
  audio.update();
  // update score
  scorer.addPoses(player_pose, avatar_pose);
  TimePoint curr_time = std::chrono::steady_clock::now();
  double elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count() * 1e-9;
  if (elapsed_time > 1) {
    prev_score_time = curr_time;
    score = scorer.getScore();
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
  // render player's pose
  if (!player_pose.empty()) {
    canvas.renderPose(camera_frame, player_pose, 75, 125, 255);
  }
  int target_height = camera_frame.rows;
  int target_width = video.width() * target_height / video.height();
  camera_frame = camera_frame(cv::Range(0, target_height),
    cv::Range(0.5 * camera_frame.cols - 0.5 * target_width, 0.5 * camera_frame.cols + 0.5 * target_width));
  // render avatar's pose
  double scalar = (double) target_height / video_frame.rows;
  cv::resize(video_frame, video_frame, cv::Size(target_width, target_height));
  if (!avatar_pose.empty()) {
    avatar_pose.transform(scalar, 0, 0);
    canvas.renderPose(video_frame, avatar_pose, 255, 0, 255);
  }
  // render pose comparison
  cv::Mat test_frame = cv::Mat(target_height, target_width, camera_frame.type(), cv::Scalar(255, 255, 255));
  if (!player_pose.empty()) {
    player_pose.standardize();
    player_pose.transform(160, 0.5 * target_width, 0.2 * target_height);
    canvas.renderPose(test_frame, player_pose, 75, 125, 255);
  }
  if (!avatar_pose.empty()) {
    avatar_pose.standardize();
    avatar_pose.transform(160, 0.5 * target_width, 0.2 * target_height);
    canvas.renderPose(test_frame, avatar_pose, 255, 0, 255);
  }
  // concatenate and show frames
  cv::Mat frame;
  vector<cv::Mat> frames = {test_frame, video_frame, camera_frame};
  cv::hconcat(frames, frame);
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}
