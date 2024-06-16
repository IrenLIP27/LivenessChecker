#include "livenessestimates.h"

#include <opencv2/imgcodecs.hpp>

#include <dlib/dnn.h>
#include <dlib/data_io.h>
#include <dlib/image_processing.h>

// ----------------------------------------------------------------------------------------
using namespace dlib;

template <long num_filters, typename SUBNET> using con5d = con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = con<num_filters,5,5,1,1,SUBNET>;

template <typename SUBNET> using downsampler  = relu<affine<con5d<32, relu<affine<con5d<32, relu<affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5  = relu<affine<con5<45,SUBNET>>>;

using net_type = loss_mmod<con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

// ----------------------------------------------------------------------------------------

bool LivenessEstimates::checkCrop(const cv::Mat& frame) {
    auto primaryRect = m_faceRect;

    if(primaryRect.x <= m_settings.margin || primaryRect.y <= m_settings.margin ||
        primaryRect.x + primaryRect.width >= frame.cols - m_settings.margin ||
        primaryRect.y + primaryRect.height >= frame.rows - m_settings.margin) {
        return true;
    } else {
        return false;
    }
}

bool LivenessEstimates::analyzeHeadPose() {
    auto headPoseEst = m_headPoseEst.analyze_head_pose(m_shape);
    bool yawOk;
    bool pitchOk;
    bool rollOk;

    if(fabs(headPoseEst.yaw) <= m_settings.yawThreshold)
        yawOk = true;
    else
        yawOk = false;

    if(fabs(headPoseEst.pitch) <= m_settings.pitchThreshold)
        pitchOk = true;
    else
        pitchOk = false;

    if(fabs(headPoseEst.roll) <= m_settings.rollThreshold)
        rollOk = true;
    else
        rollOk = false;

    if(yawOk && pitchOk && rollOk)
        return true;
    else
        return false;
}

bool LivenessEstimates::analyzeEyes() {
    auto res = m_headPoseEst.analyze_eyes(m_shape);

    if(res) {
        if((res.leftEye.state != LivenessSDK::EyeAttributes::State::Open)
            or (res.rightEye.state != LivenessSDK::EyeAttributes::State::Open)) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool LivenessEstimates::imageQuality(const cv::Mat& frame) {
    auto res = m_qualityEst.compute(frame);

    bool light = (res[0] / 100.f) < m_settings.lightThreshold;
    bool dark = (res[1] / 100.f) < m_settings.darkThreshold;
    bool blur = (res[2] / 100.f) < m_settings.blurThreshold;

    if(light) {
        return false;
    }
    if(dark) {
        return false;
    }
    if(blur) {
        return false;
    }

    return true;
}

LivenessEstimates::LivenessEstimates()
{
    m_detector = dlib::get_frontal_face_detector();
    dlib::deserialize("./shape_predictor_5_face_landmarks.dat") >> m_sp;
}


LivenessEstimates::DetectFaceRes LivenessEstimates::detectFace(const cv::Mat& frame)
{
    dlib::array2d<dlib::bgr_pixel> dlibFrame;
    dlib::assign_image(dlibFrame, dlib::cv_image<dlib::bgr_pixel>(frame));
    auto faces = m_detector(dlibFrame);

    if(faces.empty()) {
        return {};
    }
    m_faceRect = faces[0];

    bool sizeOK = m_faceRect.width > m_settings.m_minWidth && m_faceRect.height > m_settings.m_minHeight;

    if(!sizeOK) {
        DetectFaceRes DetectFaceRes;
        DetectFaceRes.errMessage = "Слишком маленькое лицо";
        return DetectFaceRes;
    }

    m_shape = m_sp(dlibFrame, m_faceRect);
    dlib::extract_image_chip(dlibFrame, dlib::get_face_chip_details(m_shape, 150, 0.25), m_faceWrap);

    bool scoreOK = m_faceWrap.score >= m_settings.qualityThreshold;
    if(!scoreOK) {
        DetectFaceRes DetectFaceRes;
        DetectFaceRes.errMessage = "Скор детекции лица слишком маленький";
        return DetectFaceRes;
    }

    cv::Rect faceRect(m_faceRect.x,
                      m_faceRect.y,
                      m_faceRect.width,
                      m_faceRect.height);

    DetectFaceRes detectFaceRes;
    detectFaceRes.face = faceRect;
    return detectFaceRes;
}

LivenessEstimates::LivenessRes LivenessEstimates::livenessChecks(const cv::Mat& frame)
{
    auto cropped = checkCrop(frame);
    if(cropped) {
        LivenessRes LivenessRes;
        LivenessRes.errMessage = "Лицо слишком близко к краям экрана";
        return LivenessRes;
    }

    auto analyzeHeadPoseRes = analyzeHeadPose();
    if(!analyzeHeadPoseRes) {
        LivenessRes LivenessRes;
        LivenessRes.errMessage = "Некорректное положение головы";
        return LivenessRes;
    }

    auto analyzeEyesRes = analyzeEyes();
    if(!analyzeEyesRes) {
        LivenessRes LivenessRes;
        LivenessRes.errMessage = "Оба глаза должны быть открытыми";
        return LivenessRes;
    }

    auto imageQualityRes = imageQuality(frame);
    if(!imageQualityRes) {
        LivenessRes LivenessRes;
        LivenessRes.errMessage = "Низкое качество кадра";
        return LivenessRes;
    }

    auto livenessEstRes = m_livenessEst.check_liveness(m_faceWrap, frame);
    if(livenessEstRes.state != LivenessSDK::LState::Alive) {
        LivenessRes LivenessRes;
        LivenessRes.errMessage = "Liveness проверка не пройдена";
        LivenessRes.color = "red";
        return LivenessRes;
    }

    LivenessRes livenessRes;
    livenessRes.isOk = true;
    return livenessRes;
}

QString LivenessEstimates::getBestShot()
{
    auto faceWrapMat = dlib::toMat(m_faceWrap);
    std::vector<uint8_t> buffer;
    cv::imencode(".jpeg", faceWrapMat, buffer);
    QByteArray byteArray = QByteArray::fromRawData((const char*)buffer.data(), buffer.size());
    QString base64Image(byteArray.toBase64());
    base64Image = "data:image/jpeg;base64," + base64Image;
    return base64Image;
}
