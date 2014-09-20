#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(),
                                          qApp->desktop()->availableGeometry()));
    this->setWindowTitle("SORDE");

    ui->objectList->AdjustToContents;

    svmDataDirectory = "data/Train_SVM/";
    calibDataDirectory = "data/Calibration/";

    stereoCameraDialog = new StereoCameraDialog(this, calibDataDirectory);
    int mode = stereoCameraDialog->exec();
    if(mode == QDialog::Accepted)
    {
        this->leftCamera = stereoCameraDialog->getLeftCameraIdx();
        this->rightCamera = stereoCameraDialog->getRightCameraIdx();
    }
    else
    {
        this->leftCamera = 0;
        this->rightCamera = 1;
    }

    this->map_l1 = stereoCameraDialog->getMap_l1();
    this->map_l2 = stereoCameraDialog->getMap_l2();
    this->map_r1 = stereoCameraDialog->getMap_r1();
    this->map_r2 = stereoCameraDialog->getMap_r2();
    this->Q = stereoCameraDialog->getQ();

    loadSamples();
    populateList();

    progressBar = new QProgressBar(ui->statusBar);
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->hide();

    captureLeft = cv::VideoCapture(this->leftCamera);
    if(!captureLeft.isOpened())
    {
        captureLeft.open(this->leftCamera);
    }
    captureRight = cv::VideoCapture(this->rightCamera);
    if(!captureRight.isOpened())
    {
        captureRight.open(this->rightCamera);
    }
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    timer->start(10);

    categorizerThread = NULL;
    calibrationThread = NULL;
    dictionaryThread = NULL;
    disparityThread = NULL;

}

MainWindow::~MainWindow()
{
    captureLeft.release();
    captureRight.release();
    delete ui;
}


void MainWindow::updateFrame()
{
    captureLeft.grab();
    captureRight.grab();

    captureLeft.retrieve(currentFrameLeft);
    captureRight.retrieve(currentFrameRight);

    if(!currentFrameLeft.empty() && !currentFrameRight.empty())
    {
        if(categorizerThread == NULL || (categorizerThread != NULL && categorizerThread->isFinished()))

        {
            findObjects();
        }


        if(ui->tabWidget->currentIndex() == 0)
        {
            showDetectedObjects(currentFrameLeft);
            frameLeft = MatToQImage(currentFrameLeft);
            ui->leftCameraLabel->setPixmap(QPixmap::fromImage(frameLeft).scaled(currentFrameLeft.cols, currentFrameLeft.rows, Qt::KeepAspectRatio));
        }
        else
        {
            showDetectedObjects(currentFrameRight);
            frameRight = MatToQImage(currentFrameRight);
            ui->rightCameraLabel->setPixmap(QPixmap::fromImage(frameRight).scaled(currentFrameRight.cols, currentFrameRight.rows, Qt::KeepAspectRatio));
        }
   }


}

void MainWindow::loadSamples()
{
    loadDictionary(this, svmDataDirectory, vocab, svms);
    loadTemplateImages(svmDataDirectory, categoryNames, templates);
    generateTemplateFeatures(this, svmDataDirectory, categoryNames, templates, keypoints, desc);
}

void MainWindow::populateList()
{
    ui->availableObjectsList->addItems(categoryNames);
}

int MainWindow::getCheckedItem()
{
    if(ui->objectList->count() > 0)
    {
        for(int row = 0; row < ui->objectList->count(); row++)
        {
            if(ui->objectList->item(row)->checkState() == Qt::Checked)
            {
                return row;
            }
        }
    }
    return -1;
}

void MainWindow::showDetectedObjects(cv::Mat frame)
{
    // Display detected objects
    if(!detectedObjects.isEmpty())
    {
        QMap<QString, std::vector<cv::Point2f>>::iterator iter;
        cv::RNG rng(12345);
        for(iter = detectedObjects.begin(); iter != detectedObjects.end(); iter++)
        {
            cv::Scalar color(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawRectangle(frame, iter.value(), color, iter.key());
        }
    }
}

void MainWindow::findObjects()
{
    if(templates.isEmpty() || categoryNames.isEmpty() ||
            keypoints.isEmpty() || desc.isEmpty() || svms.isEmpty() ||vocab.empty())
    {
        ui->statusBar->showMessage("No template images or template SURF features available", 1000);
    }
    else
    {
        if(detectedObjects.size() > 0)
        {
            detectedObjects.clear();
        }
        categorizerThread = new CategorizerThread(currentFrameLeft, svms, vocab,
                                                  keypoints, desc, templates, categoryNames);

        qRegisterMetaType<QMap<QString, std::vector<cv::Point2f> >>("QMap<QString, std::vector<cv::Point2f> >");

        connect(categorizerThread, SIGNAL(doneProcessing(QMap<QString, std::vector<cv::Point2f> >)),
              this, SLOT(objectRecognition(QMap<QString, std::vector<cv::Point2f> >)));
        connect(categorizerThread, SIGNAL(sendException(QString,int)), this, SLOT(setMessage(QString, int)));

        categorizerThread -> start();


    }
}

void MainWindow::objectRecognition(const QMap<QString, std::vector<cv::Point2f> > &detectedObjects)
{

    this->detectedObjects = detectedObjects;

    if(!detectedObjects.isEmpty())
    {
        ui -> distanceButton -> setEnabled(true);

        QMap<QString, std::vector<cv::Point2f>>::const_iterator iter;
        for(iter = detectedObjects.begin(); iter != detectedObjects.end(); iter++)
        {
            QList<QListWidgetItem *> lsMatch = ui->objectList->findItems(iter.key(), Qt::MatchFixedString);
            if (lsMatch.isEmpty())
            {
                QListWidgetItem* newItem = new QListWidgetItem();
                newItem->setText(iter.key());
                newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                newItem->setCheckState(Qt::Unchecked);
                ui->objectList->addItem(newItem);
            }
        }
        for(int i = 0; i < ui->objectList->count(); i++)
        {
            QListWidgetItem *item = ui->objectList->item(i);
            if(detectedObjects.find(item->text()) == detectedObjects.end())
            {
                ui->objectList->takeItem(i);
            }
        }

    }
    else
    {
        ui -> objectList -> clear();
        ui -> distanceButton -> setEnabled(false);
        ui -> distanceLine -> clear();
    }
}

void MainWindow::setProgress(int progress)
{
    progressBar->setValue(progress);
    if(progress == 100)
    {
        progressBar->hide();
    }
}

void MainWindow::setDictSVM(const QMap<QString, cv::SVM> &svms, const cv::Mat &vocab)
{
    progressBar->hide();
    this->vocab = vocab;
    this->svms = svms;
    ui->availableObjectsList->clear();
    ui->availableObjectsList->addItems(categoryNames);

}

void MainWindow::setMessage(const QString &message, int timeout)
{
    ui->statusBar->showMessage(message, timeout);
}

void MainWindow::setObjectDistance(const cv::Scalar &distance, const QString &category)
{
    QString objectDistance = category + ": " + QString::number(distance[0]/10.0) + " cm";
    ui->distanceLine->setText(objectDistance);

}

void MainWindow::setRectificationData(const cv::Mat &map_l1, const cv::Mat &map_l2, const cv::Mat &map_r1, const cv::Mat &map_r2, const cv::Mat &Q)
{
    this->map_l1 = map_l1;
    this->map_l2 = map_l2;
    this->map_r1 = map_r1;
    this->map_r2 = map_r2;
    this->Q = Q;
}

void MainWindow::drawRectangle(cv::Mat img,
                               std::vector<cv::Point2f> corners, cv::Scalar color, QString category)
{

    cv::RotatedRect rect;
    rect = cv::minAreaRect(corners);
    cv::Point2f points[4];
    rect.points(points);

    cv::line( img, points[0], points[1], color, 4 );
    cv::line( img, points[1], points[2], color, 4 );
    cv::line( img, points[2], points[3], color, 4 );
    cv::line( img, points[3], points[0], color, 4 );

    cv::putText(img, category.toStdString(), (points[0] + points[2]) * 0.5,
            cv::FONT_HERSHEY_DUPLEX, 1, color, 1);
}

void MainWindow::on_distanceButton_clicked()
{
    if(map_l1.empty() || map_l2.empty() || map_r1.empty() || map_r2.empty() || Q.empty())
    {
        QMessageBox::critical(this, "No Calibration Data Found", "No calibration data found. Try loading from file, if available(File->Load Calibration Data) or calibrating your stereo camera(File->Camera Calibration)");
    }
    else
    {
       int row = getCheckedItem();
       if(!detectedObjects.empty() && row != -1)
       {     
          QString object = ui->objectList->item(row)->text();

          disparityThread = new DisparityThread(currentFrameLeft, currentFrameRight, map_l1, map_l2,
                                                map_r1, map_r2, Q, currentFrameLeft.size(),
                                                detectedObjects[object], object);
          qRegisterMetaType<cv::Scalar>("cv::Scalar");
          connect(disparityThread, SIGNAL(objectDistance(cv::Scalar, QString)), this, SLOT(setObjectDistance(cv::Scalar, QString)));
          connect(disparityThread, SIGNAL(sendMessage(QString,int)), this, SLOT(setMessage(QString,int)));

          disparityThread->start();

       }

    }
}

void MainWindow::on_actionLoad_Dictionary_triggered()
{
    QString dataDirectoryName = QFileDialog::getExistingDirectory(this, tr("Select Data Directory"), dataDirectoryName,
                                                             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!dataDirectoryName.isEmpty())
    {
        dataDirectoryName += "/";
        loadDictionary(this, dataDirectoryName, vocab, svms);
    }

}

void MainWindow::on_actionGenerate_Template_Keypoints_triggered()
{

    QString dataDirectoryName = QFileDialog::getExistingDirectory(this, tr("Select Data Directory"), dataDirectoryName,
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!dataDirectoryName.isEmpty())
    {
        dataDirectoryName += "/";
        loadTemplateImages(dataDirectoryName, categoryNames, templates);
        generateTemplateFeatures(this, dataDirectoryName, categoryNames, templates, keypoints, desc);
    }
}

void MainWindow::on_actionAdd_object_triggered()
{
    dictDialog = new DictionaryDialog(this, leftCamera, svmDataDirectory);

    timer -> stop();
    dictDialog -> show();

    int mode = dictDialog -> exec();
    if(mode == QDialog::Rejected)
    {
        timer -> start();
    }
    if(mode == QDialog::Accepted)
    {
        if(!categoryNames.contains(dictDialog -> getObjectName()))
        {
            categoryNames.push_back(dictDialog -> getObjectName());
        }
        templates[dictDialog -> getObjectName()] = dictDialog -> getObjectTemplate();
        std::vector<cv::KeyPoint> kp;
        cv::Mat descriptor;
        generateKpDesc(dictDialog -> getObjectTemplate(), kp, descriptor);
        keypoints[dictDialog -> getObjectName()] = kp;
        desc[dictDialog -> getObjectName()] = descriptor;
        timer -> start();

        dictionaryThread = new DictionaryThread(svmDataDirectory, templates, desc, categoryNames, 1000);
        connect(dictionaryThread, SIGNAL(updateProgress(int)), this, SLOT(setProgress(int)));

        qRegisterMetaType<QMap<QString, cv::SVM>>("QMap<QString, cv::SVM>");
        qRegisterMetaType<cv::Mat>("cv::Mat");
        connect(dictionaryThread, SIGNAL(doneGeneratingDictionary(QMap<QString,cv::SVM>,cv::Mat)),
                this, SLOT(setDictSVM(QMap<QString,cv::SVM>,cv::Mat)));

        progressBar->show();
        dictionaryThread->start();

    }

}

void MainWindow::on_actionCamera_Calibration_triggered()
{
    timer->stop();
    stereoCalibDialog = new StereoCalibrationDialog(this, leftCamera, rightCamera, calibDataDirectory);

    //Put the dialog in the screen center
    const QRect screen = QApplication::desktop()->screenGeometry();
    stereoCalibDialog->move(screen.topLeft());

    stereoCalibDialog->show();
    int mode = stereoCalibDialog->exec();

    if(mode == QDialog::Rejected)
    {
        timer->start();
    }
    if(mode == QDialog::Accepted)
    {
        cv::Size patternSize = stereoCalibDialog->getPatternSize();
        float sideLength = stereoCalibDialog->getSquareSize();

        timer->start();

        calibrationThread = new CalibrationThread(calibDataDirectory, patternSize, sideLength);

        connect(calibrationThread, SIGNAL(updateProgress(int)), this, SLOT(setProgress(int)));
        connect(calibrationThread, SIGNAL(sendMessage(QString)), ui->statusBar, SLOT(showMessage(QString)));
        qRegisterMetaType<cv::Mat>("cv::Mat");
        connect(calibrationThread, SIGNAL(sendRectificationData(cv::Mat,cv::Mat,cv::Mat,cv::Mat,cv::Mat)),
                this, SLOT(setRectificationData(cv::Mat,cv::Mat,cv::Mat,cv::Mat,cv::Mat)));

        progressBar->show();
        calibrationThread->start();

    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionLoad_Calibration_Data_triggered()
{
    timer->stop();
    QString calibDataFile = QFileDialog::getOpenFileName(this, "Select Calibration Data File",
                                                         calibDataDirectory, "XML (*.xml);;All Files (*)");

    timer->start();
    if(!calibDataFile.isEmpty())
    {
        cv::FileStorage fs(calibDataFile.toStdString(), cv::FileStorage::READ);
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

}

void MainWindow::on_actionHelp_triggered()
{
    QFileInfo helpFile("data/help.pdf");
    if(helpFile.exists())
    {
        QDesktopServices::openUrl(QUrl("file:///" + helpFile.absoluteFilePath()));
    }
}
