#include "pose.h"

PoseEstimator::PoseEstimator() {
    net = cv::dnn::readNetFromTensorflow("graph_opt.pb");
}

Pose PoseEstimator::estimate(cv::Mat& frame) {
    net.setInput(cv::dnn::blobFromImage(frame, 1.0, cv::Size(net_width, net_height), cv::Scalar(127.5, 127.5, 127.5), true));
    cv::Mat out = net.forward();

    int result_width = out.size[3];
    int result_height = out.size[2];

    Pose pose;
    for(int i = 0; i < body_parts.size(); i++) {
        // Slice heatmap of corresponding body's part.
        cv::Mat heatMap(result_height, result_width, CV_32F, out.ptr(0, i));
        // 1 maximum per heatmap
        cv::Point point(-1,-1);
        cv::Point point_max;
        double confidence;
        cv::minMaxLoc(heatMap, 0, &confidence, 0, &point_max);
        if (confidence > min_confidence) {
            point = point_max;
        }
        pose.body_parts[body_parts[i]] = Point((double)point.x / result_width, (double)point.y / result_height);
    }
    return pose;
}