#include "categorizerthread.h"

#include <QDebug>

CategorizerThread::CategorizerThread(cv::Mat frame, QMap<QString, cv::SVM> svms,
                                     cv::Mat vocab,
                                     QMap<QString, std::vector<cv::KeyPoint>> keypoints,
                                     QMap<QString, cv::Mat> desc,
                                     QMap<QString, cv::Mat> templates,
                                     QList<QString> categoryNames) :
    QThread()
{
    this->frame = frame;
    this->desc = desc;
    this->svms = svms;
    this->vocab = vocab;
    this->keypoints = keypoints;
    this->templates = templates;
    this->categoryNames = categoryNames;

    categories = this->categoryNames.size();

    featureDetector = new cv::SurfFeatureDetector(500);
    descriptorExtractor = new cv::SurfDescriptorExtractor();
    descriptorMatcher = new cv::FlannBasedMatcher();
    bowDescriptorExtractor = new cv::BOWImgDescriptorExtractor(descriptorExtractor, descriptorMatcher);
    bowDescriptorExtractor->setVocabulary(this->vocab);

    doStop = false;
}

CategorizerThread::~CategorizerThread()
{
    delete featureDetector;
    delete descriptorExtractor;
    delete descriptorMatcher;
    delete bowDescriptorExtractor;
}


void CategorizerThread::run()
{
    doStopMutex.lock();
    if(doStop)
    {
        doStop = false;
        doStopMutex.unlock();
    }
    doStopMutex.unlock();

    processingMutex.lock();

    std::vector<cv::KeyPoint> kp_frame;
    cv::Mat bowDescriptor;
    cv::Mat frame_g;
    cv::Mat desc_frame;
    std::vector<QString> predictedCategories;

    cv::cvtColor(frame, frame_g, CV_BGR2GRAY);

    //Extract frame BOW descriptor and SURF descriptor
    featureDetector -> detect(frame_g, kp_frame);
    descriptorExtractor ->compute(frame_g, kp_frame, desc_frame);
    bowDescriptorExtractor -> compute(frame_g, kp_frame, bowDescriptor);

    // Predict using SVMs for all categories, choose the prediction with the most negative signed distance measure
    if(predictedCategories.size() > 0) {
        predictedCategories.clear();
    }

    for(int i = 0; i < categories; i++) {
        QString category = categoryNames[i];

        //cv::redirectError(handleError);
        if(svms.contains(category))
        {
            try
            {
                float prediction = svms[category].predict(bowDescriptor, true);
                if(prediction < 0.5)
                {
                   predictedCategories.push_back(category);
                }
            }
            catch(cv::Exception e)
            {
                emit sendException(QString::fromStdString(e.err), 2500);
            }
        }
    }

    std::vector<QString>::iterator iter;
    if(predictedCategories.size() > 0)
    {
        for(iter = predictedCategories.begin(); iter != predictedCategories.end(); iter++)
        {
            objectRecognition(kp_frame, desc_frame, *iter);
        }
    }

    processingMutex.unlock();

    // Inform GUI thread of detected objects
    emit doneProcessing(detectedObjects);

}
void CategorizerThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop = true;
}

void CategorizerThread::objectRecognition(std::vector<cv::KeyPoint> kpFrame, cv::Mat descFrame, QString category)
{
   std::vector<std::vector<cv::DMatch>> matches;
   std::vector<cv::Point2f> obj;
   std::vector<cv::Point2f> scene;
   std::vector<cv::Point2f> obj_corners(4);
   std::vector<cv::Point2f> scene_corners(4);
   cv::Mat H;
   int goodMatchesCounter;

   std::vector<cv::KeyPoint> kpTemplate = keypoints[category];
   cv::Mat descTemplate = desc[category];

   obj_corners[0] = cv::Point(0, 0);
   obj_corners[1] = cv::Point(templates[category].cols, 0);
   obj_corners[2] = cv::Point(templates[category].cols, templates[category].rows);
   obj_corners[3] = cv::Point(0, templates[category].rows);

   try
   {
       descriptorMatcher -> knnMatch(descTemplate, descFrame, matches, 2);
   }
   catch(cv::Exception e)
   {
       emit sendException(QString::fromStdString(e.err), 2500);
   }

   goodMatchesCounter = 0;
   for(int i = 0; i < std::min(descFrame.rows - 1, (int)matches.size()); i++)
   {
       if((matches[i][0].distance < 0.6*(matches[i][1].distance)) && (matches[i].size() && matches[i].size() > 0 ))
       {
           obj.push_back(kpTemplate[matches[i][0].queryIdx].pt);
           scene.push_back(kpFrame[matches[i][0].trainIdx].pt);
           goodMatchesCounter++;
       }
   }

   if(goodMatchesCounter >= 4)
   {
       H = cv::findHomography(obj, scene, CV_RANSAC);
       cv::perspectiveTransform(obj_corners, scene_corners, H);
       detectedObjects[category] = scene_corners;

   }
}
