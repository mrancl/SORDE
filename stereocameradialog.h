#ifndef STEREOCAMERADIALOG_H
#define STEREOCAMERADIALOG_H

//Qt
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

//OpenCV
#include <opencv2/opencv.hpp>

namespace Ui {
class StereoCameraDialog;
}

class StereoCameraDialog : public QDialog
{
    Q_OBJECT

public:

    explicit StereoCameraDialog(QWidget *parent = 0, QString calibDataDirectory = "");
    ~StereoCameraDialog();

    int getLeftCameraIdx() const;
    void setLeftCameraIdx(int value);

    int getRightCameraIdx() const;
    void setRightCameraIdx(int value);

    cv::Mat getMap_l1() const;
    void setMap_l1(const cv::Mat &value);

    cv::Mat getMap_l2() const;
    void setMap_l2(const cv::Mat &value);

    cv::Mat getMap_r1() const;
    void setMap_r1(const cv::Mat &value);

    cv::Mat getMap_r2() const;
    void setMap_r2(const cv::Mat &value);

    cv::Mat getQ() const;
    void setQ(const cv::Mat &value);

private slots:

    void on_buttonBox_accepted();
    void on_browseButton_clicked();

private:

    Ui::StereoCameraDialog *ui;

    int leftCameraIdx;
    int rightCameraIdx;
    QString calibDataDirectory;

    /* Needed to calculate distance(disparity) */
    cv::Mat map_l1, map_l2;
    cv::Mat map_r1, map_r2;
    cv::Mat Q;

    void loadCalibData(const QString &fileName);

};

#endif // STEREOCAMERADIALOG_H
