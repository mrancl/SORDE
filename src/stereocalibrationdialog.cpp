#include "stereocalibrationdialog.h"
#include "ui_stereocalibrationdialog.h"

#include <QDebug>

StereoCalibrationDialog::StereoCalibrationDialog(QWidget *parent, int leftCamId, int rightCamId, QString calibDir) :
    QDialog(parent),
    ui(new Ui::StereoCalibrationDialog)
{
    ui->setupUi(this);

    this->leftCamId = leftCamId;
    this->rightCamId = rightCamId;
    this->calibDir = calibDir;
    this->setWindowTitle("Stereo Camera calibration");
    addDirectory(calibDir);
    createCalibDirs();

    captureLeft = cv::VideoCapture(this->leftCamId);
    if(!captureLeft.isOpened())
    {
        captureLeft.open(this->leftCamId);
    }
    captureRight = cv::VideoCapture(this->rightCamId);
    if(!captureRight.isOpened())
    {
        captureRight.open(this->rightCamId);
    }

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

    imageCounter = 0;
    patternSize.height = 0;
    patternSize.width = 0;
    squareSize = 0.f;

}

StereoCalibrationDialog::~StereoCalibrationDialog()
{
    delete ui;
}

cv::Size StereoCalibrationDialog::getPatternSize() const
{
    return patternSize;
}

void StereoCalibrationDialog::setPatternSize(const cv::Size &value)
{
    patternSize = value;
}
float StereoCalibrationDialog::getSquareSize() const
{
    return squareSize;
}

void StereoCalibrationDialog::setSquareSize(float value)
{
    squareSize = value;
}



void StereoCalibrationDialog::createCalibDirs()
{
    addDirectory(calibDir + "Left/");
    addDirectory(calibDir + "Right/");
}

void StereoCalibrationDialog::updateFrame()
{
    captureLeft.grab();
    captureRight.grab();

    captureLeft.retrieve(currentFrameLeft);
    captureRight.retrieve(currentFrameRight);

    if(!currentFrameLeft.empty() && !currentFrameRight.empty())
    {
        cv::Mat frameLeftCopy = currentFrameLeft.clone();
        cv::Mat frameRightCopy = currentFrameRight.clone();

        if(ui->checkBox->isChecked())
        {
            detectChessboard(frameLeftCopy, patternSize);
            detectChessboard(frameRightCopy, patternSize);
        }

        QImage qframeL, qframeR;
        qframeL = MatToQImage(frameLeftCopy);
        qframeR = MatToQImage(frameRightCopy);

        frameLeft = MatToQImage(currentFrameLeft);
        frameRight = MatToQImage(currentFrameRight);

        pixmapLeft = QPixmap::fromImage(qframeL).scaled(currentFrameLeft.cols,
                                                  currentFrameRight.rows, Qt::KeepAspectRatio);

        pixmapRight = QPixmap::fromImage(qframeR).scaled(currentFrameRight.cols,
                                                  currentFrameRight.rows, Qt::KeepAspectRatio);
        ui->leftCameraLabel->setPixmap(pixmapLeft);
        ui->rightCameraLabel->setPixmap(pixmapRight);

    }

}

void StereoCalibrationDialog::on_cancelButton_clicked()
{
    reject();
}

void StereoCalibrationDialog::on_captureButton_clicked()
{
    if(timer->isActive())
    {
        QString leftFileName = calibDir + "Left/" + "left" + QString::number(++imageCounter) + ".jpg";
        QString rightFileName = calibDir + "Right/" + "right" + QString::number(imageCounter) + ".jpg";

        frameLeft.save(leftFileName);
        frameRight.save(rightFileName);

        ui->leftLine->setText("Captured image \"left" + QString::number(imageCounter) + ".jpg\"");
        ui->rightLine->setText("Captured image \"right" + QString::number(imageCounter) + ".jpg\"");
    }
}

void StereoCalibrationDialog::on_doneButton_clicked()
{
    timer->stop();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Start Camera Calibration", "You have captured " +
                                  QString::number(imageCounter) + " images. Do you wish to start calibration?",
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        if(squareSize == 0.f || patternSize.height == 0 || patternSize.width == 0)
        {
            QMessageBox::critical(this, "No chessboard data", "No chessboard pattern size or chessboard square size found. This information is necessary for calibration!");
        }
        else
        {
            accept();
        }
    }
    else
    {
        timer->start();
    }
}

void StereoCalibrationDialog::on_submitButton_clicked()
{
    QRegExp rx("[0-9 ]+");
    if(!rx.exactMatch(ui->rowsLine->text()) || !rx.exactMatch(ui->columnsLine->text()) ||
            !rx.exactMatch(ui->squareSizeLength->text()))
    {
        QMessageBox::warning(this, "Incorrect pattern size", "Please enter a valid pattern size.");
    }
    else
    {
        patternSize.height = ui->rowsLine->text().toInt();
        patternSize.width = ui->columnsLine->text().toInt();
        squareSize = ui->squareSizeLength->text().toFloat();
        QMessageBox::information(this, "Cheesboard pattern size submitted", "Chessboard pattern size submitted without issues");

        ui->leftCameraLabel->setText("");
        timer->start();
    }
}
