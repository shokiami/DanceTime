#include "defs.h"

typedef cv::Point_<double> Point;

class PoseEstimator {
    public:
    PoseEstimator();
    Pose estimate(cv::Mat& frame);

    private:
    static const int net_width = 368;
    static const int net_height = 368;
    cv::dnn::Net net;
    static constexpr double min_confidence = 0.1;
    std::vector<string> body_parts = { "Nose", "Neck", "RShoulder", "RElbow", "RWrist", "LShoulder", "LElbow", "LWrist", 
                                       "RHip", "RKnee", "RAnkle", "LHip", "LKnee", "LAnkle", "REye", "LEye", "REar", "LEar", "Background" };
};

class Pose {
    public:
    std::map<string, Point> body_parts;
};
