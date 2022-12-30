#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "pose.h"

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "pose_landmarks";
string calculator_graph_config_file = "mediapipe/pose_tracking.pbtxt";
mediapipe::CalculatorGraph* graph;
mediapipe::OutputStreamPoller* poller;
Pose pose;
bool prev_inframe;

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
  poller = new mediapipe::OutputStreamPoller(std::move(*graph->AddOutputStreamPoller(kOutputStream)));
  graph->StartRun({});
  prev_inframe = false;
}

PoseEstimator::~PoseEstimator() {
  graph->CloseInputStream(kInputStream);
  delete poller;
  delete graph;
}

Pose PoseEstimator::getPose(cv::Mat& raw_frame, bool wait) {
  // convert frame from BGR to RGB
  cv::Mat rgb_frame;
  cv::cvtColor(raw_frame, rgb_frame, cv::COLOR_BGR2RGB);

  // convert frame into an ImageFrame
  std::unique_ptr<mediapipe::ImageFrame> image_frame = absl::make_unique<mediapipe::ImageFrame>(
    mediapipe::ImageFormat::SRGB, raw_frame.cols, raw_frame.rows, mediapipe::ImageFrame::kDefaultAlignmentBoundary
  );
  rgb_frame.copyTo(mediapipe::formats::MatView(image_frame.get()));

  // send frame to graph input stream
  double start_time = (double) cv::getTickCount() / cv::getTickFrequency();
  mediapipe::Timestamp timestamp = mediapipe::Timestamp(1e6 * start_time);
  graph->AddPacketToInputStream(kInputStream, mediapipe::Adopt(image_frame.release()).At(timestamp));

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

  // build the pose from the landmark list
  pose = Pose();
  for (int i = 0; i < body_parts.size(); i++) {
    const mediapipe::NormalizedLandmark& landmark = landmark_list.landmark(i);

    // add landmark to pose if visible
    if (landmark.visibility() > 0.1) {
      pose[body_parts[i]] = Point(landmark.x() * raw_frame.cols, landmark.y() * raw_frame.rows);
    }
  }

  prev_inframe = true;
  return pose;
}
