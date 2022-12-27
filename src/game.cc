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
    if (score == 0) {
      cout << "please step into frame" << endl;
    } else {
      cout << "fps: " << fps << ", score: " << std::setprecision(2) << std::fixed << score << "%" << endl;
    }
  }

  // update fps
  elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - prev_fps_time).count() * 1e-9;
  prev_fps_time = curr_time;
  fps = 1 / elapsed_time;
}

void Game::render() {
  // prepare resizing constants
  const double video_scalar = camera_frame.rows / ((double) footer_frame.rows * video_frame.cols / footer_frame.cols + video_frame.rows);
  const int video_height = video_scalar * video_frame.rows;
  const int video_width = video_scalar * video_frame.cols;
  const int camera_height = camera_frame.rows;
  const int camera_width = 1.5 * video_width;
  const int footer_height = camera_height - video_height;
  const int footer_width = video_width;

  // crop camera frame
  // canvas.render(camera_frame, player_pose, 75, 125, 255);
  camera_frame = camera_frame(cv::Range(0, camera_height),
    cv::Range(0.5 * camera_frame.cols - 0.5 * camera_width, 0.5 * camera_frame.cols + 0.5 * camera_width));

  // resize video frame and avatar pose
  cv::resize(video_frame, video_frame, cv::Size(video_width, video_height));
  for (string body_part : avatar_pose.keys()) {
    avatar_pose[body_part] *= video_scalar;
  }
  // canvas.render(video_frame, avatar_pose, 255, 0, 255);

  // resize foot frame
  cv::resize(footer_frame, footer_frame, cv::Size(footer_width, footer_height));

  // concatenate and display frames
  cv::Mat frame;
  cv::vconcat(video_frame, footer_frame, frame);
  cv::hconcat(frame, camera_frame, frame);
  cv::imshow("DanceTime", frame);
  key_code = cv::waitKey(1) & 0xFF;
}
