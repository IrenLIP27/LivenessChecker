#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "imageprovider.h"
#include "webcamcapture.h"

#include <opencv2/core/utils/logger.hpp>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // LOG_LEVEL_ERROR, LOG_LEVEL_SILENT
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

    ImageProvider* imageProvider = new ImageProvider();
    engine.rootContext()->setContextProperty("ImageProvider", imageProvider);
    engine.addImageProvider("ImageProvider", imageProvider);

    qRegisterMetaType<QImage>("QImage");

    qmlRegisterType<WebcamCapture>("WebcamCaptureQml", 1, 0, "WebcamCapture");

    const QUrl url(u"qrc:/LivenessChecker/Main.qml"_qs);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
