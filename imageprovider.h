#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
    Q_OBJECT
    QImage img;
    bool isShowImages = true;

public:
    explicit ImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

public slots:
    void acceptImage(const QImage& some_img);
    void emptyImage();
    bool isNull() const;
    void stopVideoRunning();
    void startVideoRunning();
};

#endif // IMAGEPROVIDER_H
