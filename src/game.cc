#include "game.h"

Game::Game(string name) : capture(0), video(name), audio(name) {
  audio.play();
  video.play();
  prev_score_time = std::chrono::steady_clock::now();
  prev_fps_time = std::chrono::steady_clock::now();
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
    player1_score = scorer.score(player1_history, avatar_history);
    player2_score = scorer.score(player2_history, avatar_history);
    // print scores
    if (!scorer.inframe(player1_pose)) {
      cout << "player 1: please step into frame" << endl;
    } else if (scorer.inframe(avatar_pose)) {
      cout << "player 1: " << 1e-2 * std::round(1e4 * player1_score) << "%" << endl;
    }
    if (!scorer.inframe(player2_pose)) {
      cout << "player 2: please step into frame" << endl;
    } else if (scorer.inframe(avatar_pose)) {
      cout << "player 2: " << 1e-2 * std::round(1e4 * player2_score) << "%" << endl;
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
    canvas.render(player1_frame, player1_pose, 255, 50, 50);
  }
  player1_frame = player1_frame(cv::Range(0, player1_frame.rows),
    cv::Range(0, player1_frame.cols - video_width / 2));

  // resize player2 frame
  if (debug) {
    canvas.render(player2_frame, player2_pose, 75, 125, 255);
  }
  player2_frame = player2_frame(cv::Range(0, player2_frame.rows),
    cv::Range(video_width / 2, player2_frame.cols));

  // resize video frame
  cv::resize(video_frame, video_frame, cv::Size(video_width, video_height));
  if (debug) {
    for (string body_part : avatar_pose.keys()) {
      avatar_pose[body_part] *= video_scalar;
    }
    canvas.render(video_frame, avatar_pose, 255, 0, 255);
  }

  // resize footer frame
  cv::resize(footer_frame, footer_frame, cv::Size(video_width, player1_frame.rows - video_height));

  // concatenate frames into single frame
  cv::Mat frame;
  cv::vconcat(vector<cv::Mat>{video_frame, footer_frame}, frame);
  cv::hconcat(vector<cv::Mat>{player1_frame, frame, player2_frame}, frame);

  // render fps
  if (debug) {
    cv::putText(frame, "fps: " + std::to_string(fps), cv::Point(15, 45), 0, 1.0, CV_RGB(100, 200, 0), 2);
  }

  // display frame
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}
