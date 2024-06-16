#include "webcamcapture.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

WebcamCapture::WebcamCapture(QObject *parent) : QObject(parent) {
    m_isWorkerNeedRunning = false;
    m_isGotBestShot = false;
}

void WebcamCapture::start() {
    if(m_thread != nullptr) {
        emit sessionAlreadyRunning();
        return;
    }

    m_thread = QThread::create([this](){
        cv::VideoCapture cap(0);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        if(!cap.isOpened()) {
            QString messageStr = QString("{\"message_from_cpp\": \"%1\", \"color_from_cpp\": \"%2\"}").
                                 arg(QString::fromStdString("Не удалось подключиться к веб-камере")).arg(QString::fromStdString("red"));
            emit sendMessage(messageStr);
            QThread::currentThread()->exit(0);
            return 1;
        }
        m_isWorkerNeedRunning = true;
        cv::Mat frame;
        QImage qImgRes;
        int frameNum = 0;

        for(;;) {
            if(!m_isWorkerNeedRunning) {
                break;
            }
            cap.read(frame);
            ++frameNum;

            if(frame.empty()) {
                QString messageStr = QString("{\"message_from_cpp\": \"%1\", \"color_from_cpp\": \"%2\"}").
                                     arg(QString::fromStdString("Ошибка при получении кадра с веб-камеры")).arg(QString::fromStdString("red"));
                emit sendMessage(messageStr);
                break;
            }

            if((frameNum % 3) != 0) {
                continue;
            }

            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            if(!m_isGotBestShot) {
                auto detectFaceRes = m_livenessChecks.detectFace(frame);

                if(!detectFaceRes.errMessage.empty()) {
                    QString messageStr = QString("{\"message_from_cpp\": \"%1\", \"color_from_cpp\": \"%2\"}").
                                         arg(QString::fromStdString(detectFaceRes.errMessage)).arg(QString::fromStdString(detectFaceRes.color));
                    emit sendMessage(messageStr);
                }

                if(!detectFaceRes.face.empty()) {
                    auto livenessRes = m_livenessChecks.livenessChecks(frame);
                    cv::rectangle(frame, detectFaceRes.face, cv::Scalar(255, 255, 0), 1, cv::LINE_8);

                    if(livenessRes.isOk) {
                        m_isGotBestShot = true;
                        emit set_bestshot(m_livenessChecks.getBestShot());
                    }

                    if(!livenessRes.errMessage.empty()) {
                        QString messageStr = QString("{\"message_from_cpp\": \"%1\", \"color_from_cpp\": \"%2\"}").
                                             arg(QString::fromStdString(livenessRes.errMessage)).arg(QString::fromStdString(livenessRes.color));
                        emit sendMessage(messageStr);
                    }
                }
            }

            QImage qImg((uchar*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
            qImgRes = qImg.copy();
            emit imgReady(qImgRes);
        }
        emit worker_thread_stoped();
        QThread::currentThread()->exit(0);
        return 0;
    });

    if(!m_thread) {
        return;
    }

    m_thread->start();
    emit worker_thread_started();
}

void WebcamCapture::stop() {
    m_isWorkerNeedRunning = false;
    if(m_thread != nullptr) {
        m_thread->wait();
        m_thread->deleteLater();
        m_thread = nullptr;
    }
    m_isGotBestShot = false;
}
