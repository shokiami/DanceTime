#include "game.h"

Game::Game(string name) : capture(0), video(name), audio(name) {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
  player1_score = 0;
  player2_score = 0;
  player1_total_score = 0;
  player2_total_score = 0;
  seconds_passed = 0;
  debug = false;
}

bool Game::finished() {
  return key_code == 'q' || video.finished();
}

void Game::update() {
  // get camera frame
  cv::Mat camera_frame;
  capture.read(camera_frame);
  if (camera_frame.empty()) {
    ERROR("empty frame from camera");
  }
  cv::flip(camera_frame, camera_frame, 1);

  // get player1 frame
  player1_frame = camera_frame(cv::Range(0, camera_frame.rows), cv::Range(0, camera_frame.cols / 2));

  // get player2 frame
  player2_frame = camera_frame(cv::Range(0, camera_frame.rows), cv::Range(camera_frame.cols / 2, camera_frame.cols));

  // get video frame
  video_frame = video.currFrame();
  if (video_frame.empty()) {
    ERROR("empty frame from video");
  }

  // get footer frame
  footer_frame = video.currFooterFrame();
  if (footer_frame.empty()) {
    ERROR("empty frame from footer");
  }

  // get poses
  player1_pose = pose_estimator.getPose1(player1_frame);
  player2_pose = pose_estimator.getPose2(player2_frame);
  avatar_pose = video.getPose();

  // update pose histories
  player1_history.push_back(player1_pose);
  player2_history.push_back(player2_pose);
  avatar_history.push_back(avatar_pose);

  // update audio
  audio.update();

  // update scores
  TimePoint curr_time = std::chrono::steady_clock::now();
  double elapsed_score_time = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count();
  if (elapsed_score_time >= 1) {
    prev_score_time = curr_time;
    if (scorer.inframe(avatar_pose)) {
      player1_score = 100.0 * scorer.score(player1_history, avatar_history);
      player2_score = 100.0 * scorer.score(player2_history, avatar_history);
      player1_total_score += player1_score;
      player2_total_score += player2_score;
      seconds_passed++;
    }
    // clear histories
    player1_history.clear();
    player2_history.clear();
    avatar_history.clear();
  }

  // update fps
  double elapsed_fps_time = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count();
  prev_fps_time = curr_time;
  fps = 1.0 / elapsed_fps_time;

  // toggle debug mode
  if (key_code == ' ') {
    debug = !debug;
  }
}

void Game::render() {
  // calculate video frame resize
  const double video_scalar = player1_frame.rows / ((double) footer_frame.rows * video_frame.cols / footer_frame.cols + video_frame.rows);
  const int video_height = video_scalar * video_frame.rows;
  const int video_width = video_scalar * video_frame.cols;

  // resize player1 frame
  if (debug) {
    canvas.render_pose(player1_frame, player1_pose, 255, 50, 50);
  }
  player1_frame = player1_frame(cv::Range(0, player1_frame.rows),
    cv::Range(0, player1_frame.cols - video_width / 2));

  // resize player2 frame
  if (debug) {
    canvas.render_pose(player2_frame, player2_pose, 75, 125, 255);
  }
  player2_frame = player2_frame(cv::Range(0, player2_frame.rows),
    cv::Range(video_width / 2, player2_frame.cols));

  // resize video frame
  cv::resize(video_frame, video_frame, cv::Size(video_width, video_height));
  if (debug) {
    for (string body_part : avatar_pose.keys()) {
      avatar_pose[body_part] *= video_scalar;
    }
    canvas.render_pose(video_frame, avatar_pose, 255, 0, 255);
  }

  // resize footer frame
  cv::resize(footer_frame, footer_frame, cv::Size(video_width, player1_frame.rows - video_height));

  // warn players if out of frame
  double warning_x = player1_frame.cols - 425.0;
  double warning_y = player1_frame.rows / 2 - 10.0;
  if (!scorer.inframe(player1_pose)) {
    canvas.render_text(player1_frame, "please step into frame", warning_x, warning_y, 255, 235, 0);
  }
  if (!scorer.inframe(player2_pose)) {
    canvas.render_text(player2_frame, "please step into frame", warning_x, warning_y, 255, 235, 0);
  }

  // render scores
  double player1_progress = 0.01 * player1_total_score / video.totalTime();
  double player2_progress = 0.01 * player2_total_score / video.totalTime();
  canvas.render_score(player1_frame, player1_score, player1_progress, 255, 50, 50, false);
  canvas.render_score(player2_frame, player2_score, player2_progress, 75, 125, 255, true);

  // concatenate frames into single frame
  cv::Mat frame;
  cv::vconcat(vector<cv::Mat>{video_frame, footer_frame}, frame);
  cv::hconcat(vector<cv::Mat>{player1_frame, frame, player2_frame}, frame);

  // render fps
  if (debug) {
    canvas.render_text(frame, "fps: " + std::to_string(fps), 15.0, 45.0, 100, 200, 0);
  }

  // display frame
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}

pair<double, double> Game::results() {
  double player1_final_score = (double) player1_total_score / seconds_passed;
  double player2_final_score = (double) player2_total_score / seconds_passed;
  return {player1_final_score, player2_final_score};
}
