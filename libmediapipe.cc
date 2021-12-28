#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/formats/landmark.pb.h"

class PoseEstimator;
class Pose;
class Landmark;

class PoseEstimator {
  public:
  PoseEstimator();
  ~PoseEstimator();
  Pose getPose(cv::Mat& frame, bool wait = false);

  private:
  std::vector<std::string> body_parts = 
    { "nose", "right eye (inner)", "right eye", "right eye (outer)", "left eye (inner)", "left eye", "left eye (outer)", 
      "right ear", "left ear", "mouth (right)", "mouth (left)", "right shoulder", "left shoulder", "right elbow", "left elbow", 
      "right wrist", "left wrist", "right pinky", "left pinky", "right index", "left index", "right thumb", "left thumb", 
      "right hip", "left hip", "right knee", "left knee", "right ankle", "left ankle", "right heel", "left heel", "right foot index", "left foot index" };
};

class Pose {
  public:
  void addLandmark(Landmark landmark);

  private:
  std::vector<Landmark> landmarks;
};

class Landmark {
  public:
  Landmark(std::string body_part, cv::Point3d position, double visibility, double presence);

  private:
  std::string body_part;
  cv::Point3d position;
  double visibility;
  double presence;
};

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "pose_landmarks";
std::string calculator_graph_config_file = "graphs/pose_tracking.pbtxt";
mediapipe::CalculatorGraph* graph;
mediapipe::OutputStreamPoller* landmark_poller;
Pose pose;
bool out_of_frame;

PoseEstimator::PoseEstimator() {
  std::cout << "Getting calculator graph configuration." << std::endl;
  std::string calculator_graph_config_contents;
  mediapipe::file::GetContents(calculator_graph_config_file, &calculator_graph_config_contents);
  mediapipe::CalculatorGraphConfig config =
    mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(calculator_graph_config_contents);
  std::cout << "Initializing calculator graph." << std::endl;
  graph = new mediapipe::CalculatorGraph();
  graph->Initialize(config);
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
  // If there is a new landmark packet, then update pose.
  // Or if 'wait', then wait until the graph returns a new landmark packet.
  if (landmark_poller->QueueSize() > 0 || wait) {
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
    if (out_of_frame) {
      // If out of frame, set the pose to empty.
      pose = Pose();
    }
    out_of_frame = true;
  }
  return pose;
}
