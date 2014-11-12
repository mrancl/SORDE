#ifndef CATEGORIZERTHREAD_H
#define CATEGORIZERTHREAD_H

//Qt
#include <QThread>
#include <vector>
#include <QMap>
#include <QMutex>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/ml/ml.hpp>

class CategorizerThread : public QThread
{
    Q_OBJECT
public:

    CategorizerThread(cv::Mat frame, QMap<QString, cv::SVM> svms, cv::Mat vocab,
                      QMap<QString, std::vector<cv::KeyPoint> > keypoints, QMap<QString, cv::Mat> desc,
                      QMap<QString, cv::Mat> templates, QList<QString> categoryNames);
    void stop();

    cv::Mat getFrame() const;
    void setFrame(const cv::Mat &value);

private:

    volatile bool doStop;
    QMutex doStopMutex;
    QMutex processingMutex;

    cv::Mat frame;
    QMap<QString, cv::Mat> templates;
    QMap<QString, cv::SVM> svms; //trained SVMs, mapped by category name
    int categories; //number of categories
    cv::Mat vocab; //vocabulary
    QMap<QString, std::vector<cv::KeyPoint> > keypoints; //map of template keypoints
    QMap<QString, cv::Mat> desc; //map of template descriptors
    QMap<QString, std::vector<cv::Point2f> > detectedObjects;
    QList<QString> categoryNames;


    // Feature detectors and descriptor extractors
    cv::Ptr<cv::FeatureDetector> featureDetector;
    cv::Ptr<cv::DescriptorExtractor> descriptorExtractor;
    cv::Ptr<cv::BOWImgDescriptorExtractor> bowDescriptorExtractor;
    cv::Ptr<cv::FlannBasedMatcher> descriptorMatcher;

    void objectRecognition(std::vector<cv::KeyPoint> kpFrame, cv::Mat descFrame, QString category); //recognize object

protected:

    void run(); //function to perform real-time object categorization on camera frames

signals:

    void sendException(const QString &err, int timeout);
    void doneProcessing(const QMap<QString, std::vector<cv::Point2f> > &detectedObjects);

};

#endif // CATEGORIZERTHREAD_H
