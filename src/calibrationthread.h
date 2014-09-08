#ifndef CALIBRATIONTHREAD_H
#define CALIBRATIONTHREAD_H

//Qt
#include <QThread>
#include <QDir>
#include <QDirIterator>
#include <QVector>
#include <Qlist>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

//Local
#include "utilities.h"

#include <vector>

class CalibrationThread : public QThread
{
    Q_OBJECT
public:

    CalibrationThread(QString calibDir, cv::Size patternSize, float sideLength);
    void stop();

private:

    int progressCounter;

    volatile bool doStop;
    QMutex doStopMutex;
    QMutex processingMutex;

    QString calibDir;

    QList<cv::Mat> imagesLeft; // Chessboard images
    QList<cv::Mat> imagesRight;
    cv::Mat cameraMatrixLeft, cameraMatrixRight;
    cv::Mat distCoeffsLeft, distCoeffsRight;

    float sideLength; //side length of a chessboard square in mm
    cv::Size patternSize; //number of internal corners of the chessboard along width and height
    std::vector<std::vector<cv::Point2f>> imagePointsLeft, imagePointsRight; // 2D image points
    std::vector<std::vector<cv::Point3f>> objectPoints; // 3D object points

    cv::Mat R, T, E, F; //stereo calibration information
    cv::Mat map_l1, map_l2, map_r1, map_r2; //pixel maps for rectification
    cv::Mat Q;

    void loadImages(QString camera, QList<cv::Mat> &images);
    void calcImagePoints(std::vector<std::vector<cv::Point2f>> &imagePoints, QList<cv::Mat> &images);
    void calcImagePointsStereo();
    void singleCameraCalibration(QString camera, cv::Mat &cameraMatrix, cv::Mat &distCoeffs,
                                 std::vector<std::vector<cv::Point2f>> imagePoints, QList<cv::Mat> images);
    void stereoCalibration();
    void rectifyImage();

protected:

    void run(); // calibrate the camera

signals:

    void updateProgress(int);
    void sendMessage(const QString &rmsError);
    void sendRectificationData(const cv::Mat &map_l1, const cv::Mat &map_l2, const cv::Mat &map_r1,
                               const cv::Mat &map_r2, const cv::Mat &Q);


};

#endif // CALIBRATIONTHREAD_H
