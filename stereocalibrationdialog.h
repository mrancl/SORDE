#ifndef STEREOCALIBRATIONDIALOG_H
#define STEREOCALIBRATIONDIALOG_H

//Qt
#include <QDialog>
#include <QTimer>
#include <QImage>
#include <QMessageBox>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

//Local
#include "utilities.h"

namespace Ui {
class StereoCalibrationDialog;
}

class StereoCalibrationDialog : public QDialog
{
    Q_OBJECT

public:

    explicit StereoCalibrationDialog(QWidget *parent = 0,
                                     int leftCamId = 0, int rightCamId = 0, QString calibDir = "data/Calibration/");
    ~StereoCalibrationDialog();

    cv::Size getPatternSize() const;
    void setPatternSize(const cv::Size &value);

    float getSquareSize() const;
    void setSquareSize(float value);

private:

    Ui::StereoCalibrationDialog *ui;

    QString calibDir;

    int leftCamId;
    int rightCamId;

    cv::Mat currentFrameLeft; // Current frame in cv::Mat format
    cv::Mat currentFrameRight; // Current frame in cv::Mat format
    QImage frameLeft; // Current frame in QImage format
    QImage frameRight; // Current frame in QImage format

    QTimer *timer;
    cv::VideoCapture captureLeft;
    cv::VideoCapture captureRight;
    QPixmap pixmapLeft;
    QPixmap pixmapRight;
    int imageCounter;

    cv::Size patternSize;
    float squareSize;

    void createCalibDirs();

private slots:

    void updateFrame();
    void on_cancelButton_clicked();
    void on_captureButton_clicked();
    void on_doneButton_clicked();
    void on_submitButton_clicked();
};

#endif // STEREOCALIBRATIONDIALOG_H
