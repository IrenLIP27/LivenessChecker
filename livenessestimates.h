#ifndef LIVENESSESTIMATES_H
#define LIVENESSESTIMATES_H

#include <QString>

#include <opencv2/core.hpp>

#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>

#include "LivenessSDK/include"

class LivenessEstimates
{
public:
    struct LivenessRes {
        bool isOk = false;
        std::string errMessage;
        std::string color = "white";
    };

    struct DetectFaceRes {
        cv::Rect face;
        std::string errMessage;
        std::string color = "white";
    };

    LivenessEstimates();

    DetectFaceRes detectFace(const cv::Mat& frame);
    LivenessRes livenessChecks(const cv::Mat& frame);
    QString getBestShot();

private:
    bool checkCrop(const cv::Mat& frame);
    bool analyzeHeadPose();
    bool analyzeEyes();
    bool imageQuality(const cv::Mat& frame);

    struct Settings {
        int m_minWidth = 70;
        int m_minHeight = 100;
        float qualityThreshold = 0.6f;
        int margin = 10;
        int yawThreshold = 15;
        int pitchThreshold = 15;
        int rollThreshold = 10;
        float lightThreshold = 0.9f;
        float darkThreshold = 0.93f;
        float blurThreshold = 0.94f;
    };

    dlib::frontal_face_detector m_detector;
    dlib::shape_predictor m_sp;
    dlib::rectangle m_faceRect;
    dlib::matrix<dlib::bgr_pixel> m_faceWrap;
    dlib::full_object_detection m_shape;
    Settings m_settings;
    LivenessSDK::HeadPoseEstimator m_headPoseEst;
    cv::quality::QualityBRISQUE::QualityBRISQU m_qualityEst;
    LivenessSDK::LivenessEstimator m_livenessEst;
};

#endif // LIVENESSESTIMATES_H
