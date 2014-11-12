#ifndef DICTIONARYTHREAD_H
#define DICTIONARYTHREAD_H

//Qt
#include <QThread>
#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QDir>
#include <QDirIterator>
#include <QMutex>
#include <QMutexLocker>

//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>

class DictionaryThread : public QThread
{
    Q_OBJECT
public:

    DictionaryThread(QString dataDir, QMap<QString, cv::Mat> templates,
                     QMap<QString, cv::Mat> desc,
                     QList<QString> categoryNames,
                     int clusters);

    void stop();

    QMap<QString, cv::Mat> getTemplates() const;
    void setTemplates(const QMap<QString, cv::Mat> &value);

    QList<QString> getCategoryNames() const;
    void setCategoryNames(const QList<QString> &value);

    QMap<QString, cv::Mat> getDesc() const;
    void setDesc(const QMap<QString, cv::Mat> &value);

private:

    int progressCounter;

    volatile bool doStop;
    QMutex doStopMutex;
    QMutex processingMutex;

    QString dataDir;
    QMap<QString, cv::Mat> templates;
    QMultiMap<QString, QString> trainSet;
    QMap<QString, cv::Mat> positiveData;
    QMap<QString, cv::Mat> negativeData;
    QList<QString> categoryNames;
    QMap<QString, cv::Mat> desc;
    cv::Mat vocab;
    QMap<QString, cv::SVM> svms;

    cv::Ptr<cv::FeatureDetector> featureDetector;
    cv::Ptr<cv::DescriptorExtractor> descriptorExtractor;
    cv::Ptr<cv::FlannBasedMatcher> descriptorMatcher;
    cv::Ptr<cv::BOWKMeansTrainer> bowtrainer;
    cv::Ptr<cv::BOWImgDescriptorExtractor> bowDescriptorExtractor;

    void makeTrainSet(); //method to build the training set multimap
    void makePosNeg(); //method to extract BOW features from training images and organize them into positive and negative samples
    void buildVocab(); //method to build the BOW vocabulary
    void trainClassifiers(); //method to train the one-vs-all SVM classifiers for all categories

    void findFilesRecursively(QDir rootDir);

protected:

    void run();

signals:

    void doneGeneratingDictionary(const QMap<QString, cv::SVM> &svms, const cv::Mat &vocab);
    void updateProgress(int progress);

};

#endif // DICTIONARYTHREAD_H
