#ifndef DISPARITYTHREAD_H
#define DISPARITYTHREAD_H

//Qt
#include <QThread>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

class DisparityThread : public QThread
{
    Q_OBJECT
public:

    DisparityThread(cv::Mat framel, cv::Mat framer, cv::Mat map_l1, cv::Mat map_l2, cv::Mat map_r1, cv::Mat map_r2,
                    cv::Mat Q, cv::Size imageSize, std::vector<cv::Point2f> detectedObject, QString category);
    void stop();

private:

    volatile bool doStop;
    QMutex doStopMutex;
    QMutex processingMutex;

    cv::Mat framel;
    cv::Mat framer;
    cv::Mat map_l1, map_l2;
    cv::Mat map_r1, map_r2;
    cv::Mat Q;

    QString category;

    std::vector<cv::Point2f> detectedObject;

    cv::Size imageSize;

    cv::StereoSGBM stereo;

protected:

    void run();

signals:

    void objectDistance(const cv::Scalar &distance, const QString &category);
    void sendMessage(const QString &message, int timeout);

};

#endif // DISPARITYTHREAD_H
