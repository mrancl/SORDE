#include "calibrationthread.h"
#include <windows.h>

#include <QDebug>

CalibrationThread::CalibrationThread(QString calibDir, cv::Size patternSize, float sideLength)
{
    this->calibDir = calibDir;
    this->patternSize = patternSize;
    this->sideLength = sideLength;

    progressCounter = 0;

    doStop = false;
}

void CalibrationThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop = true;
}

void CalibrationThread::run()
{
    doStopMutex.lock();
    if(doStop)
    {
        doStop = false;
        doStopMutex.unlock();
    }
    doStopMutex.unlock();

    processingMutex.lock();

    //Right camera calibration
    loadImages("Right", imagesRight);
    calcImagePoints(imagePointsRight, imagesRight);
    singleCameraCalibration("right", cameraMatrixRight, distCoeffsRight, imagePointsRight, imagesRight);
    progressCounter  = 25;
    emit updateProgress(progressCounter);

    objectPoints.clear();

    //Left camera calibration
    loadImages("Left", imagesLeft);
    calcImagePoints(imagePointsLeft, imagesLeft);
    singleCameraCalibration("left", cameraMatrixLeft, distCoeffsLeft, imagePointsLeft, imagesLeft);
    progressCounter  = 50;
    emit updateProgress(progressCounter);

    objectPoints.clear();
    imagePointsLeft.clear();
    imagePointsRight.clear();

    //Stereo calibration
    calcImagePointsStereo();
    stereoCalibration();
    progressCounter  = 90;
    emit updateProgress(progressCounter);

    //Stereo rectification
    rectifyImage();
    progressCounter  = 100;
    emit updateProgress(progressCounter);

    processingMutex.unlock();

    //Inform GUI of rectification data
    emit sendRectificationData(map_l1, map_l2, map_r1, map_r2, Q);


}

void CalibrationThread::loadImages(QString camera, QList<cv::Mat> &images)
{

    QDir calibDirectory(calibDir + camera);
    calibDirectory.setFilter(QDir::Files | QDir::NoDot | QDir::NoDotDot);
    QDirIterator it(calibDirectory, QDirIterator::Subdirectories);

    while(it.hasNext())
    {
        it.next();
        images.push_back(cv::imread(it.fileInfo().absoluteFilePath().toStdString()));
    }

}

void CalibrationThread::calcImagePoints(std::vector<std::vector<cv::Point2f> > &imagePoints,
                                        QList<cv::Mat> &images)
{
    // Calculate the object points in the object co-ordinate system (origin at top left corner)
    std::vector<cv::Point3f> objPoints;
    for(int i = 0; i < patternSize.height; i++)
    {
        for(int j = 0; j < patternSize.width; j++)
        {
            objPoints.push_back(cv::Point3f(j * sideLength, i * sideLength, 0.f));
        }
    }

    for(int i = 0; i < images.size(); i++)
    {
        cv::Mat im = images[i];
        std::vector<cv::Point2f> imPoints;

        //Find chessboard corners
        bool patternFound = cv::findChessboardCorners(im, patternSize, imPoints,
                                                      cv::CALIB_CB_ADAPTIVE_THRESH +
                                                      cv::CALIB_CB_NORMALIZE_IMAGE +
                                                      cv::CALIB_CB_FAST_CHECK);
        if(patternFound)
        {
            objectPoints.push_back(objPoints);
            cv::Mat gray;
            cv::cvtColor(im, gray, CV_BGR2GRAY);
            cv::cornerSubPix(gray, imPoints, cv::Size(5,5), cv::Size(-1, -1),
                             cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            imagePoints.push_back(imPoints);
        }

    }
}

void CalibrationThread::calcImagePointsStereo()
{
    // Calculate the object points in the object co-ordinate system (origin at top left corner)
    std::vector<cv::Point3f> objPoints;
    for(int i = 0; i < patternSize.height; i++)
    {
        for(int j = 0; j < patternSize.width; j++)
        {
            objPoints.push_back(cv::Point3f(j * sideLength, i * sideLength, 0.f));
        }
    }

    for(int i = 0; i < imagesLeft.size(); i++)
    {
        cv::Mat lim = imagesLeft[i];
        cv::Mat rim = imagesRight[i];

        std::vector<cv::Point2f> imPointsLeft, imPointsRight;

        bool patternFoundLeft = cv::findChessboardCorners(lim, patternSize, imPointsLeft,
                                                      cv::CALIB_CB_ADAPTIVE_THRESH +
                                                      cv::CALIB_CB_NORMALIZE_IMAGE +
                                                      cv::CALIB_CB_FAST_CHECK);
        bool patternFoundRight = cv::findChessboardCorners(rim, patternSize, imPointsRight,
                                                      cv::CALIB_CB_ADAPTIVE_THRESH +
                                                      cv::CALIB_CB_NORMALIZE_IMAGE +
                                                      cv::CALIB_CB_FAST_CHECK);
        if(patternFoundLeft && patternFoundRight)
        {
            objectPoints.push_back(objPoints);
            cv::Mat gray;
            cv::cvtColor(lim, gray, CV_BGR2GRAY);
            cv::cornerSubPix(gray, imPointsLeft, cv::Size(5,5), cv::Size(-1, -1),
                             cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            cv::cvtColor(rim, gray, CV_BGR2GRAY);
            cv::cornerSubPix(gray, imPointsRight, cv::Size(5,5), cv::Size(-1, -1),
                             cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            imagePointsLeft.push_back(imPointsLeft);
            imagePointsRight.push_back(imPointsRight);
        }
        else
        {
            imagesLeft.erase(imagesLeft.begin() + i);
            imagesRight.erase(imagesRight.begin() + i);
        }
    }

}

void CalibrationThread::singleCameraCalibration(QString camera, cv::Mat &cameraMatrix, cv::Mat &distCoeffs,
                                                std::vector<std::vector<cv::Point2f> > imagePoints, QList<cv::Mat> images)
{
    std::vector<cv::Mat> rvecs, tvecs;

    float rmsError = cv::calibrateCamera(objectPoints, imagePoints, images[0].size(),
                                         cameraMatrix, distCoeffs, rvecs, tvecs);

    QString fileName = calibDir + camera + "_calib.xml";
    cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release();
    emit sendMessage("RMS reprojection error of " + QString::number(rmsError) + " for " + camera + " camera");
}

void CalibrationThread::stereoCalibration()
{
    if(!cameraMatrixLeft.empty() && !distCoeffsLeft.empty() && !cameraMatrixRight.empty() && !distCoeffsRight.empty())
    {
        double rms = cv::stereoCalibrate(objectPoints, imagePointsLeft, imagePointsRight,
                                         cameraMatrixLeft, distCoeffsLeft, cameraMatrixRight, distCoeffsRight,
                                         imagesLeft[0].size(), R, T, E, F);
        QString fileName = calibDir + "stereo_calib.xml";
        cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
        fs << "cameraMatrixLeft" << cameraMatrixLeft;
        fs << "cameraMatrixRight" << cameraMatrixRight;
        fs << "distCoeffsLeft" << distCoeffsLeft;
        fs << "distCoeffsRight" << distCoeffsRight;
        fs << "R" << R;
        fs << "T" << T;
        fs << "E" << E;
        fs << "F" << F;
        fs.release();

        emit sendMessage("RMS reprojection error of " + QString::number(rms) + " for stereo camera");
    }
}

void CalibrationThread::rectifyImage()
{
    if(cameraMatrixLeft.empty() || cameraMatrixRight.empty() || distCoeffsLeft.empty() ||
       distCoeffsRight.empty() || R.empty() || T.empty())
    {
        emit sendMessage("No calibration data");
    }
    else
    {

        // Calculate transforms for rectifying images
        cv::Mat Rl, Rr, Pl, Pr;
        cv::Size imageSize = imagesLeft[0].size();
        cv::stereoRectify(cameraMatrixLeft, distCoeffsLeft, cameraMatrixRight,
                          distCoeffsRight, imageSize, R, T, Rl, Rr, Pl, Pr, Q);

        // Calculate pixel maps for efficient rectification of images via lookup tables
        cv::initUndistortRectifyMap(cameraMatrixLeft, distCoeffsLeft, Rl, Pl, imageSize,
                                    CV_16SC2, map_l1, map_l2);
        cv::initUndistortRectifyMap(cameraMatrixRight, distCoeffsRight, Rr, Pr, imageSize,
                                    CV_16SC2, map_r1, map_r2);

        QString fileName = calibDir + "stereo_calib.xml";
        cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::APPEND);
        fs << "Rl" << Rl;
        fs << "Rr" << Rr;
        fs << "Pl" << Pl;
        fs << "Pr" << Pr;
        fs << "Q" << Q;
        fs << "map_l1" << map_l1;
        fs << "map_l2" << map_l2;
        fs << "map_r1" << map_r1;
        fs << "map_r2" << map_r2;
        fs.release();

        emit sendMessage("Finished stereo rectification");
    }

}

