#include "stereocameradialog.h"
#include "ui_stereocameradialog.h"

StereoCameraDialog::StereoCameraDialog(QWidget *parent, QString calibDataDirectory) :
    QDialog(parent),
    ui(new Ui::StereoCameraDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Set Camera Ids");
    ui->leftCameraLine->setText("0");
    ui->rightCameraLine->setText("1");
    this->calibDataDirectory = calibDataDirectory;
    ui->filePathLine->setText(calibDataDirectory + "stereo_calib.xml");
}

void StereoCameraDialog::on_buttonBox_accepted()
{
    QRegExp rx("[0-9 ]+");
    if(!rx.exactMatch(ui->leftCameraLine->text()) || !rx.exactMatch(ui->rightCameraLine->text()))
    {
        QMessageBox::warning(this, "Incorrect stereo camera parameters", "Please enter a valid indices.");
    }
    else
    {
        leftCameraIdx = ui->leftCameraLine->text().toInt();
        rightCameraIdx = ui->rightCameraLine->text().toInt();
        if(!ui->filePathLine->text().isEmpty())
        {
            loadCalibData(ui->filePathLine->text());
        }
        else
        {
            QMessageBox::warning(this, "No calibration data", "No calibration data found.");
        }
    }
}

void StereoCameraDialog::on_browseButton_clicked()
{
    QString calibDataFile = QFileDialog::getOpenFileName(this, "Select Calibration Data File",
                                                         calibDataDirectory, "XML (*.xml);;All Files (*)");

    if(!calibDataFile.isEmpty())
    {
        ui->filePathLine->setText(calibDataFile);

    }
}

void StereoCameraDialog::loadCalibData(const QString &fileName)
{
    cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::READ);
    fs["Q"] >> Q;
    fs["map_l1"] >> map_l1;
    fs["map_l2"] >> map_l2;
    fs["map_r1"] >> map_r1;
    fs["map_r2"] >> map_r2;
    fs.release();

    if(!map_l1.empty() && !map_l2.empty() && !map_r1.empty() && !map_r2.empty() && !Q.empty())
    {
        QMessageBox::information(this, "Calibration data", "Calibration data loaded successfully.");
    }
}

StereoCameraDialog::~StereoCameraDialog()
{
    delete ui;
}
int StereoCameraDialog::getLeftCameraIdx() const
{
    return leftCameraIdx;
}

void StereoCameraDialog::setLeftCameraIdx(int value)
{
    leftCameraIdx = value;
}
int StereoCameraDialog::getRightCameraIdx() const
{
    return rightCameraIdx;
}

void StereoCameraDialog::setRightCameraIdx(int value)
{
    rightCameraIdx = value;
}

cv::Mat StereoCameraDialog::getQ() const
{
    return Q;
}

void StereoCameraDialog::setQ(const cv::Mat &value)
{
    Q = value;
}

cv::Mat StereoCameraDialog::getMap_r2() const
{
    return map_r2;
}

void StereoCameraDialog::setMap_r2(const cv::Mat &value)
{
    map_r2 = value;
}

cv::Mat StereoCameraDialog::getMap_r1() const
{
    return map_r1;
}

void StereoCameraDialog::setMap_r1(const cv::Mat &value)
{
    map_r1 = value;
}

cv::Mat StereoCameraDialog::getMap_l2() const
{
    return map_l2;
}

void StereoCameraDialog::setMap_l2(const cv::Mat &value)
{
    map_l2 = value;
}

cv::Mat StereoCameraDialog::getMap_l1() const
{
    return map_l1;
}

void StereoCameraDialog::setMap_l1(const cv::Mat &value)
{
    map_l1 = value;
}

