#ifndef UTILITIES_H
#define UTILITIES_H

#include <QImage>
#include <QDataStream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/ml/ml.hpp>

#include <vector>


QImage MatToQImage(const cv::Mat& mat); //Convert opencv matrix to qimage
cv::Mat QImageToMat(const QImage &src); //Convert QImage to cv::Mat
void loadDictionary(QWidget *parent, QString dataDirName, cv::Mat &vocab, QMap<QString, cv::SVM> &svms); //Load dictionary
void loadTemplateImages(QString dataDirName, QList<QString> &categoryNames, QMap<QString, cv::Mat> &templates);
void generateTemplateFeatures(QWidget *parent, QString dataDirName, QList<QString> categoryNames, QMap<QString, cv::Mat> templates,
                              QMap<QString, std::vector<cv::KeyPoint> > &keypoints,
                              QMap<QString, cv::Mat> &desc);

void generateKpDesc(cv::Mat img, std::vector<cv::KeyPoint> &kp, cv::Mat &desc);
void addDirectory(QString dirName);

void detectChessboard(const cv::Mat &frame, cv::Size patternSize); //Function to detect and draw chessboard corners

#endif // UTILITIES_H
