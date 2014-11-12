#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//Qt
#include <QMainWindow>
#include <QTimer>
#include <QMap>
#include <QProgressBar>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QMetaType>
#include <QDesktopWidget>
#include <QUrl>
#include <QStyle>

#include <string>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/ml/ml.hpp>

//Local
#include "categorizerthread.h"
#include "utilities.h"
#include "dictionarydialog.h"
#include "dictionarythread.h"
#include "stereocalibrationdialog.h"
#include "calibrationthread.h"
#include "disparitythread.h"
#include "stereocameradialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

     MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;

    int leftCamera;
    int rightCamera;

    cv::Mat currentFrameLeft;
    cv::Mat currentFrameRight;
    QImage frameLeft;
    QImage frameRight;
    QTimer *timer;
    cv::VideoCapture captureLeft;
    cv::VideoCapture captureRight;

    QMap<QString, std::vector<cv::Point2f> > detectedObjects;
    QMap<QString, cv::Mat> templates;
    QMap<QString, cv::SVM> svms; //trained SVMs, mapped by category name
    int categories; //number of categories
    cv::Mat vocab; //vocabulary
    QMap<QString, std::vector<cv::KeyPoint> > keypoints; //map of template keypoints
    QMap<QString, cv::Mat> desc; //map of template descriptors
    QList<QString> categoryNames;

    QString svmDataDirectory;
    QString calibDataDirectory;

    CategorizerThread *categorizerThread;
    DictionaryThread *dictionaryThread;
    CalibrationThread *calibrationThread;
    DisparityThread *disparityThread;

    DictionaryDialog *dictDialog;
    StereoCalibrationDialog *stereoCalibDialog;
    StereoCameraDialog *stereoCameraDialog;

    QProgressBar *progressBar;

    /* Needed to calculate distance(disparity) */
    cv::Mat map_l1, map_l2;
    cv::Mat map_r1, map_r2;
    cv::Mat Q;

    void findObjects();
    void drawRectangle(cv::Mat img, std::vector<cv::Point2f> corners, cv::Scalar color, QString category); //draw rectangle around detected object

    void loadSamples();
    void populateList();
    int getCheckedItem();
    void showDetectedObjects(cv::Mat frame);

private slots:

    void updateFrame();
    void objectRecognition(const QMap<QString, std::vector<cv::Point2f> > &detectedObjects);
    void setProgress(int progress);
    void setDictSVM(const QMap<QString, cv::SVM> &svms, const cv::Mat &vocab);
    void setMessage(const QString &message, int timeout = 0);
    void setObjectDistance(const cv::Scalar &distance, const QString &category);
    void setRectificationData(const cv::Mat &map_l1, const cv::Mat &map_l2, const cv::Mat &map_r1,
                              const cv::Mat &map_r2, const cv::Mat &Q);
    void on_actionLoad_Dictionary_triggered();
    void on_actionGenerate_Template_Keypoints_triggered();
    void on_distanceButton_clicked();
    void on_actionAdd_object_triggered();
    void on_actionCamera_Calibration_triggered();
    void on_actionExit_triggered();
    void on_actionLoad_Calibration_Data_triggered();
    void on_actionHelp_triggered();
};

#endif // MAINWINDOW_H
