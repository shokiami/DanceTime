#include "camera.h"
#include "pose.h"

int Camera::main()
{
    cv::VideoCapture videoCapture(0);
    cv::Mat frame;
    PoseEstimator pose_estimator;
    while ((cv::waitKey(1) & 0xFF) != 'q') {
        videoCapture.read(frame);
        cv::flip(frame, frame, 1);
        imshow("WebCam", frame);
        Pose pose = pose_estimator.estimate(frame);
        for (const auto& [body_part, point] : pose.body_parts) {
            std::cout << body_part << ": " << point << std::endl;
        }
    }
    return 0;
}