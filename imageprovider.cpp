#include "imageprovider.h"

ImageProvider::ImageProvider()
    :  QQuickImageProvider(QQmlImageProviderBase::Image)
{}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    if(!isShowImages || img.isNull()) {
        return QImage(":/qml_images/video.png");
    }
    return img;
}

void ImageProvider::acceptImage(const QImage& some_img) {
    img = some_img;
    if(!isShowImages) {
        emptyImage();
    }
}

void ImageProvider::emptyImage() {
    img = QImage{};
}

bool ImageProvider::isNull() const {
    return img.isNull();
}

void ImageProvider::stopVideoRunning() {
    isShowImages = false;
    emptyImage();
}

void ImageProvider::startVideoRunning() {
    isShowImages = true;
}
