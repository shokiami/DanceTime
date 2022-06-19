#include "game.h"

Game::Game() : capture(0), video("blackpink"), audio("blackpink"), test_file("test/right_hand.csv") {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
  test_file << "player_x,player_y,avatar_x,avatar_y" << endl;
}

bool Game::finished() {
  return keyCode == 'q' || video.finished();
}

void Game::update() {
  // get player's pose
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("Empty frame from camera.");
  }
  cv::flip(camera_frame, camera_frame, 1);
  player_pose = pose_estimator.getPose(camera_frame);
  // get avatar's pose
  video_frame = video.getFrame();
  if (video_frame.empty()) {
    ERROR("Empty frame from video.");
  }
  avatar_pose = video.getPose();
  if (!player_pose.empty() && !avatar_pose.empty()) {
    Landmark l1 = player_pose.getLandmark("right wrist");
    Landmark l2 = avatar_pose.getLandmark("right wrist");
    test_file << l1.x << "," << l1.y << "," << l2.x << "," << l2.y << endl;
  }
  // update audio
  audio.update();
  // update score
  scorer.addPoses(player_pose, avatar_pose);
  time_point curr_time = std::chrono::steady_clock::now();
  double elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count() * 1e-9;
  if (elapsed_time > 1) {
    prev_score_time = curr_time;
    score = scorer.getScore();
    scorer.reset();
    // print fps and score
    cout << "FPS: " << fps << " Score: " << score << endl;
  }
  // update fps
  elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count() * 1e-9;
  prev_fps_time = curr_time;
  fps = 1 / elapsed_time;
}

void Game::render() {
  // render player's pose
  if (!player_pose.empty()) {
    canvas.renderPose(camera_frame, player_pose, cv::Scalar(255, 125, 75));
  }
  int camera_height = camera_frame.size[0];
  int camera_width = camera_frame.size[1];
  int target_width = video.width() * camera_height / video.height();
  camera_frame = camera_frame(cv::Range(0, camera_height),
    cv::Range(camera_width / 2 - target_width / 2, camera_width / 2 + target_width / 2));
  // render avatar's pose
  resize(video_frame, video_frame, cv::Size(target_width, camera_height));
  if (!avatar_pose.empty()) {
    canvas.renderPose(video_frame, avatar_pose, cv::Scalar(255, 0, 255));
  }
  // concatenate and show frames
  cv::Mat frame;
  cv::hconcat(video_frame, camera_frame, frame);
  cv::imshow("DanceTime", frame);
  keyCode = cv::waitKey(1) & 0xFF;
}
