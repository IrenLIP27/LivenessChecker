#ifndef WEBCAMCAPTURE_H
#define WEBCAMCAPTURE_H

#include <QObject>
#include <QThread>
#include <QImage>

#include "livenessestimates.h"

class WebcamCapture: public QObject
{
    Q_OBJECT

private:
    QThread* m_thread = nullptr;
    std::atomic<bool> m_isWorkerNeedRunning;
    std::atomic<bool> m_isGotBestShot;
    LivenessEstimates m_livenessChecks;

public:
    explicit WebcamCapture(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void imgReady(const QImage& some_img);
    void sendMessage(QString message);
    void worker_thread_stoped();
    void worker_thread_started();
    void set_bestshot(QString base64Image);
    void sessionAlreadyRunning();
};

#endif // WEBCAMCAPTURE_H
