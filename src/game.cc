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

  // add poses to histories
  player_history.push_back(player_pose);
  avatar_history.push_back(avatar_pose);

  // update audio
  audio.update();

  // update fps
  TimePoint curr_time = std::chrono::steady_clock::now();
  double elapsed_time = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count();
  prev_fps_time = curr_time;
  fps = 1 / elapsed_time;

  // update score
  elapsed_time = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_score_time).count();
  if (elapsed_time >= 1) {
    prev_score_time = curr_time;
    score = scorer.score(player_history, avatar_history);
    // print fps and score
    if (!scorer.inframe(player_pose)) {
      cout << "please step into frame" << endl;
    } else if (scorer.inframe(avatar_pose)) {
      cout << "score: " << 1e-2 * std::round(1e4 * score) << "%" << endl;
    }
    // clear histories
    player_history.clear();
    avatar_history.clear();
  }
}

void Game::render() {
  // toggle debug mode
  if (key_code == ' ') {
    debug = !debug;
  }

  // prepare resizing constants
  const double video_scalar = camera_frame.rows / ((double) footer_frame.rows * video_frame.cols / footer_frame.cols + video_frame.rows);
  const int video_height = video_scalar * video_frame.rows;
  const int video_width = video_scalar * video_frame.cols;
  const int camera_height = camera_frame.rows;
  const int camera_width = 1.5 * video_width;
  const int footer_height = camera_height - video_height;
  const int footer_width = video_width;
  const int test_height = camera_height;
  const int test_width = camera_width;

  // camera frame
  if (debug) {
    canvas.render(camera_frame, player_pose, 75, 125, 255);
  }
  camera_frame = camera_frame(cv::Range(0, camera_height),
    cv::Range(0.5 * camera_frame.cols - 0.5 * camera_width, 0.5 * camera_frame.cols + 0.5 * camera_width));

  // video frame
  cv::resize(video_frame, video_frame, cv::Size(video_width, video_height));
  if (debug) {
    for (string body_part : avatar_pose.keys()) {
      avatar_pose[body_part] *= video_scalar;
    }
    canvas.render(video_frame, avatar_pose, 255, 0, 255);
  }

  // footer frame
  cv::resize(footer_frame, footer_frame, cv::Size(footer_width, footer_height));

  // concatenate frames into single frame
  cv::Mat frame;
  cv::vconcat(video_frame, footer_frame, frame);
  cv::hconcat(frame, camera_frame, frame);

  // render fps
  if (debug) {
    cv::putText(frame, "fps: " + std::to_string(fps), cv::Point(15, 45), 0, 1.0, CV_RGB(100, 200, 0), 2);
  }

  // display frame
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}
