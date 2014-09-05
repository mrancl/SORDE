#ifndef DICTIONARYDIALOG_H
#define DICTIONARYDIALOG_H

//Qt
#include <QDialog>
#include <Qtimer>
#include <QImage>

//OpenCV
#include <opencv2/highgui/highgui.hpp>

//Local
#include "utilities.h"
#include "selectionwidget.h"

namespace Ui {
class DictionaryDialog;
}

class DictionaryDialog : public QDialog
{
    Q_OBJECT

public:

    explicit DictionaryDialog(QWidget *parent = 0, int deviceNumber = 0, QString svmFolder = "data/Train_SVM");
    ~DictionaryDialog();

    QString getObjectName();
    cv::Mat getObjectTemplate();

private slots:

    void updateFrame();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void mousePressed();

    void on_captureButton_clicked();

    void on_doneButton_clicked();

private:

    Ui::DictionaryDialog *ui;

    QTimer *timer;
    cv::VideoCapture capture;

    int deviceNumber;
    cv::Mat currentFrame;
    QImage frame;

    SelectionWidget *selectTemplate;
    QPixmap pixmap;

    QString svmFolder;
    QString objectName;
    cv::Mat objectTemplate;

    int imageCounter;
};

#endif // DICTIONARYDIALOG_H
