#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "pose.h"

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "pose_landmarks";
std::string calculator_graph_config_file = "graphs/pose_tracking.pbtxt";
mediapipe::CalculatorGraph* graph;
mediapipe::OutputStreamPoller* landmark_poller;
Pose pose;
bool out_of_frame;

PoseEstimator::PoseEstimator() {
  // Get the calculator graph configuration.
  std::cout << "Getting calculator graph configuration." << std::endl;
  std::string calculator_graph_config_contents;
  mediapipe::file::GetContents(calculator_graph_config_file, &calculator_graph_config_contents);
  mediapipe::CalculatorGraphConfig config =
    mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(calculator_graph_config_contents);
  // Initialize the calculator graph.
  std::cout << "Initializing calculator graph." << std::endl;
  graph = new mediapipe::CalculatorGraph();
  graph->Initialize(config);
  // Start to run the calculator graph.
  std::cout << "Starting to run calculator graph." << std::endl;
  landmark_poller = new mediapipe::OutputStreamPoller(std::move(*graph->AddOutputStreamPoller(kOutputStream)));
  graph->StartRun({});
  out_of_frame = false;
}

PoseEstimator::~PoseEstimator() {
  graph->CloseInputStream(kInputStream);
}

Pose PoseEstimator::getPose(cv::Mat& raw_frame, bool wait) {
  // Convert the frame from BGR to RGB.
  cv::Mat frame;
  cv::cvtColor(raw_frame, frame, cv::COLOR_BGR2RGB);
  // Wrap the frame into an ImageFrame.
  std::unique_ptr<mediapipe::ImageFrame> input_frame = absl::make_unique<mediapipe::ImageFrame>(
    mediapipe::ImageFormat::SRGB, frame.cols, frame.rows,
    mediapipe::ImageFrame::kDefaultAlignmentBoundary);
  cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
  frame.copyTo(input_frame_mat);
  // Send the image packet into the graph.
  size_t frame_timestamp_us = (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
  graph->AddPacketToInputStream(kInputStream, 
    mediapipe::Adopt(input_frame.release()).At(mediapipe::Timestamp(frame_timestamp_us)));
  // If 'wait', then wait until the graph returns a new landmark packet or 100 milliseconds pass by.
  if (wait) {
    size_t start_time = (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e3;
    size_t current_time = start_time;
    while (landmark_poller->QueueSize() == 0 && current_time - start_time < 100) {
      // Wait.
      current_time = (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e3;
    }
  }
  // If there is a new landmark packet, then update pose.
  if (landmark_poller->QueueSize() > 0) {
    out_of_frame = false;
    // Get the landmark list.
    mediapipe::Packet landmark_packet;
    landmark_poller->Next(&landmark_packet);
    const mediapipe::NormalizedLandmarkList& landmark_list = landmark_packet.Get<mediapipe::NormalizedLandmarkList>();
    // Build the pose from the landmark list.
    pose = Pose();
    for (size_t i = 0; i < body_parts.size(); i++) {
      const mediapipe::NormalizedLandmark& landmark = landmark_list.landmark(i);
      const std::string body_part = body_parts[i];
      const cv::Point3d position(landmark.x(), landmark.y(), landmark.z());
      const double visibility = landmark.visibility();
      const double presence = landmark.presence();
      pose.addLandmark(Landmark(body_part, position, visibility, presence));
    }
  } else {
    if (wait || out_of_frame) {
      // The pose is out of frame. Set the pose to empty.
      pose = Pose();
    }
    out_of_frame = true;
  }
  return pose;
}
