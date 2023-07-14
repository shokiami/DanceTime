#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "pose.h"

mediapipe::CalculatorGraph* graph;
string calculator_graph_config_file = "mediapipe/pose_tracking.pbtxt";
string player1_input_stream = "player1_frames";
string player2_input_stream = "player2_frames";
string player1_output_stream = "player1_poses";
string player2_output_stream = "player2_poses";
mediapipe::OutputStreamPoller* player1_poller;
mediapipe::OutputStreamPoller* player2_poller;
Pose player1_pose;
Pose player2_pose;
bool player1_prev_inframe;
bool player2_prev_inframe;

PoseEstimator::PoseEstimator() {
  // initialize calculator graph
  string calculator_graph_config_contents;
  mediapipe::file::GetContents(calculator_graph_config_file, &calculator_graph_config_contents);
  mediapipe::CalculatorGraphConfig config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
    calculator_graph_config_contents
  );
  graph = new mediapipe::CalculatorGraph();
  graph->Initialize(config);

  // start running calculator graph
  player1_poller = new mediapipe::OutputStreamPoller(std::move(*graph->AddOutputStreamPoller(player1_output_stream)));
  player2_poller = new mediapipe::OutputStreamPoller(std::move(*graph->AddOutputStreamPoller(player2_output_stream)));
  graph->StartRun({});
  player1_prev_inframe = false;
  player2_prev_inframe = false;
}

PoseEstimator::~PoseEstimator() {
  graph->CloseInputStream(player1_input_stream);
  graph->CloseInputStream(player2_input_stream);
  delete player1_poller;
  delete player2_poller;
  delete graph;
}

Pose PoseEstimator::getPose1(cv::Mat& frame, bool wait) {
  return getPose(frame, wait, player1_input_stream, player1_poller, player1_pose, player1_prev_inframe);
}

Pose PoseEstimator::getPose2(cv::Mat& frame, bool wait) {
  return getPose(frame, wait, player2_input_stream, player2_poller, player2_pose, player2_prev_inframe);
}

Pose PoseEstimator::getPose(cv::Mat& frame, bool wait, string input_stream, mediapipe::OutputStreamPoller* poller, Pose& pose, bool& prev_inframe) {
  // convert frame from BGR to RGB
  cv::Mat rgb_frame;
  cv::cvtColor(frame, rgb_frame, cv::COLOR_BGR2RGB);

  // convert frame into an ImageFrame
  std::unique_ptr<mediapipe::ImageFrame> image_frame = absl::make_unique<mediapipe::ImageFrame>(
    mediapipe::ImageFormat::SRGB, frame.cols, frame.rows, mediapipe::ImageFrame::kDefaultAlignmentBoundary
  );
  rgb_frame.copyTo(mediapipe::formats::MatView(image_frame.get()));

  // send frame to graph input stream
  double start_time = (double) cv::getTickCount() / cv::getTickFrequency();
  mediapipe::Timestamp timestamp = mediapipe::Timestamp(1e6 * start_time);
  graph->AddPacketToInputStream(input_stream, mediapipe::Adopt(image_frame.release()).At(timestamp));

  if (wait) {
    // wait until output stream poller has a packet or 0.1 seconds pass by
    double curr_time = start_time;
    while (poller->QueueSize() == 0 && (curr_time - start_time) < 0.1) {
      curr_time = (double) cv::getTickCount() / cv::getTickFrequency();
    }
  }

  // if output stream poller does not have an available packet
  if (poller->QueueSize() == 0) {
    if (wait || !prev_inframe) {
      // pose is out of frame
      pose = Pose();
    }
    prev_inframe = false;
    return pose;
  }

  // get packet from output stream poller
  mediapipe::Packet packet;
  poller->Next(&packet);

  // get landmark list from packet
  const mediapipe::NormalizedLandmarkList& landmark_list = packet.Get<mediapipe::NormalizedLandmarkList>();

  // build pose from landmark list
  pose = Pose();
  for (int i = 0; i < body_parts.size(); i++) {
    // get landmark and add to pose if visible
    const mediapipe::NormalizedLandmark& landmark = landmark_list.landmark(i);
    if (landmark.visibility() > 0.1) {
      pose[body_parts[i]] = Point(landmark.x() * frame.cols, landmark.y() * frame.rows);
    }
  }

  prev_inframe = true;
  return pose;
}
