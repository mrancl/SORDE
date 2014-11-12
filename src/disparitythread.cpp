#include "disparitythread.h"

#include<QDebug>

DisparityThread::DisparityThread(cv::Mat framel, cv::Mat framer, cv::Mat map_l1,
                                 cv::Mat map_l2, cv::Mat map_r1, cv::Mat map_r2,
                                 cv::Mat Q, cv::Size imageSize,
                                 std::vector<cv::Point2f> detectedObject, QString category)
{
    this->framel = framel;
    this->framer = framer;

    this->map_l1 = map_l1;
    this->map_l2 = map_l2;
    this->map_r1 = map_r1;
    this->map_r2 = map_r2;
    this->Q = Q;

    this->category = category;

    this->imageSize = imageSize;

    this->detectedObject = detectedObject;

    stereo.preFilterCap = 63;
    stereo.SADWindowSize = 3;

    int cn = framel.channels();

    stereo.P1 = 8*cn*stereo.SADWindowSize*stereo.SADWindowSize;
    stereo.P2 = 32*cn*stereo.SADWindowSize*stereo.SADWindowSize;
    stereo.minDisparity = 16;
    stereo.numberOfDisparities = 96;
    stereo.uniquenessRatio = 10;
    stereo.speckleWindowSize = 100;
    stereo.speckleRange = 32;
    stereo.disp12MaxDiff = 1;
    stereo.fullDP = false;

    doStop = false;
}

void DisparityThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop = true;
}
cv::Mat DisparityThread::getFramel() const
{
    return framel;
}

void DisparityThread::setFramel(const cv::Mat &value)
{
    framel = value;
}
cv::Mat DisparityThread::getFramer() const
{
    return framer;
}

void DisparityThread::setFramer(const cv::Mat &value)
{
    framer = value;
}
QString DisparityThread::getCategory() const
{
    return category;
}

void DisparityThread::setCategory(const QString &value)
{
    category = value;
}
cv::Size DisparityThread::getImageSize() const
{
    return imageSize;
}

void DisparityThread::setImageSize(const cv::Size &value)
{
    imageSize = value;
}
std::vector<cv::Point2f> DisparityThread::getDetectedObject() const
{
    return detectedObject;
}

void DisparityThread::setDetectedObject(const std::vector<cv::Point2f> &value)
{
    detectedObject = value;
}






void DisparityThread::run()
{
    doStopMutex.lock();
    if(doStop)
    {
        doStop = false;
        doStopMutex.unlock();
    }
    doStopMutex.unlock();

    processingMutex.lock();

    cv::Mat frameLeftRect, frameRightRect;

    if(!framel.empty() && !framer.empty())
    {

        cv::remap(framer, frameRightRect, map_r1, map_r2, cv::INTER_LINEAR);
        cv::remap(framel, frameLeftRect, map_l1, map_l2, cv::INTER_LINEAR);

        try
        {
            cv::Mat disp, dispCompute, pointCloud;
            stereo(frameLeftRect, frameRightRect, disp);
            disp.convertTo(dispCompute, CV_32F, 1.f/16.f);

            //Calculate 3D co-ordinates from disparity image
            cv::reprojectImageTo3D(dispCompute, pointCloud, Q, true);
            float xmin = detectedObject[0].x;
            float ymin = detectedObject[0].y;
            float xmax = detectedObject[2].x;
            float ymax = detectedObject[2].y;

            // Extract depth of rectangle and inform gui of their mean
            pointCloud = pointCloud(cv::Range(ymin, ymax), cv::Range(xmin, xmax));
            cv::Mat z_roi(pointCloud.size(), CV_32FC1);
            int fromTo[] = {2, 0};
            cv::mixChannels(&pointCloud, 1, &z_roi, 1, fromTo, 1);

            processingMutex.unlock();

            //Inform GUI of distance to object
            emit objectDistance(cv::mean(z_roi), category);
        }
        catch(const cv::Exception& e)
        {
            emit sendMessage(QString::fromStdString(e.err), 2500);
        }
    }

}
