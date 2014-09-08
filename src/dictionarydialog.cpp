#include "dictionarydialog.h"
#include "ui_dictionarydialog.h"

#include <QMessageBox>
#include <QDir>

#include <QDebug>

DictionaryDialog::DictionaryDialog(QWidget *parent, int deviceNumber, QString svmFolder) :
    QDialog(parent),
    ui(new Ui::DictionaryDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Add Object");

    this->deviceNumber = deviceNumber;
    this->svmFolder = svmFolder;

    capture = cv::VideoCapture(this->deviceNumber);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    timer->start(10);

    selectTemplate = new SelectionWidget(this);
    ui->captureLayout->addWidget(selectTemplate);
    selectTemplate->setAlignment(Qt::AlignCenter);

    ui->captureButton->hide();
    ui->doneButton->hide();

    imageCounter = 0;

    connect(selectTemplate, SIGNAL(mousePressed()), this, SLOT(mousePressed()));

}

DictionaryDialog::~DictionaryDialog()
{
    capture.release();
    delete ui;
}

QString DictionaryDialog::getObjectName()
{
    return objectName;
}

cv::Mat DictionaryDialog::getObjectTemplate()
{
    return objectTemplate;
}

void DictionaryDialog::updateFrame()
{
    capture >> currentFrame;

    frame = MatToQImage(currentFrame);
    pixmap = QPixmap::fromImage(frame).scaled(currentFrame.cols,
                                              currentFrame.rows, Qt::KeepAspectRatio);
    selectTemplate->setPixmap(pixmap);
}

void DictionaryDialog::on_saveButton_clicked()
{
    if(!ui->nameLine->text().isEmpty())
    {
        selectTemplate->setObjectName(ui->nameLine->text());

        objectTemplate = selectTemplate->save(svmFolder);
        objectName = selectTemplate->getObjectName();
        if(!timer->isActive())
        {
            timer->start();
        }

        ui->nameLine->clear();
        ui->nameLabel->setText("Capture taining images: ");
        ui->nameLine->setReadOnly(true);
        ui->saveButton->hide();
        ui->captureButton->show();
        ui->doneButton->show();
        selectTemplate->discardRect();

    }
    else
    {
        QMessageBox::warning(this, tr("No object name"), tr("Please enter an object name"));
        return;

    }

}

void DictionaryDialog::on_cancelButton_clicked()
{
   ui->nameLine->setReadOnly(false);
   ui->nameLine->clear();
   ui->nameLabel->setText("Object name: ");
   ui->saveButton->show();
   ui->captureButton->hide();
   ui->doneButton->hide();
   if(!timer->isActive())
   {
       timer->start();
   }
   selectTemplate->discardRect();
   imageCounter = 0;
}

void DictionaryDialog::mousePressed()
{
    timer->stop();
}

void DictionaryDialog::on_captureButton_clicked()
{
    QString trImagesDirName = svmFolder + objectName + "/Training_Images/";
    QDir().mkdir(trImagesDirName);
    QString fileName = trImagesDirName + objectName + QString::number(++imageCounter) + ".jpg";
    selectTemplate->pixmap()->copy(selectTemplate->getSelectionRect()).save(fileName);
    ui->nameLine->setText("Captured train image " + objectName + QString::number(imageCounter) + ".jpg");

}

void DictionaryDialog::on_doneButton_clicked()
{
    if(imageCounter > 0)
    {
        timer->stop();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Add object to dictionary", "You have captured " + QString::number(imageCounter) + " training images. Do you wish to add object \"" + objectName + "\" to dictionary?",
                              QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
        {
            timer->start();
        }
        else
        {
            accept();
        }
    }
}
