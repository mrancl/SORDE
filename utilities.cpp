//Qt
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

//Local
#include "utilities.h"

QImage MatToQImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
        if(mat.type()==CV_8UC1)
        {
            // Set the color table (used to translate colour indexes to qRgb values)
            QVector<QRgb> colorTable;
            for (int i=0; i<256; i++)
                colorTable.push_back(qRgb(i,i,i));
            // Copy input Mat
            const uchar *qImageBuffer = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
            img.setColorTable(colorTable);
            return img;
        }
        // 8-bits unsigned, NO. OF CHANNELS=3
        else if(mat.type()==CV_8UC3)
        {
            // Copy input Mat
            const uchar *qImageBuffer = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return img.rgbSwapped();
        }
        else
        {
            qDebug() << "ERROR: Mat could not be converted to QImage.";
            return QImage();
        }
}


cv::Mat QImageToMat(QImage const& src)
{
     cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
     cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
     cv::cvtColor(tmp, result, CV_BGR2RGB);
     return result;
}

void loadDictionary(QWidget *parent, QString dataDirName, cv::Mat &vocab, QMap<QString, cv::SVM> &svms)
{
    QString vocabFileName = dataDirName + "vocab.xml";
    if(!vocabFileName.isEmpty())
    {
        QFile vocabFile(vocabFileName);
        if(!vocabFile.exists())
        {
            QMessageBox::critical(parent, "Error", "Could not find dictionary file");
        }
        else
        {
            cv::FileStorage fs(vocabFileName.toStdString(), cv::FileStorage::READ);
            fs["vocabulary"] >> vocab;
            fs.release();
        }
    }

    QDir dataDirectory(dataDirName);
    foreach (QFileInfo info, dataDirectory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::AllDirs ))
    {
        if(info.isFile() && info.baseName().contains("SVM"))
        {
            QString category = info.baseName();
            //category = category.left(1).toUpper() + category.mid(1);
            category.truncate(category.size() - 3);
            svms[category].load(info.absoluteFilePath().toStdString().c_str());

        }
    }
    if(svms.isEmpty())
    {
        QMessageBox::critical(parent, "Error", "Could not find SVM information.");
    }

}


void loadTemplateImages(QString dataDirName, QList<QString> &categoryNames, QMap<QString, cv::Mat> &templates)
{
    cv::Mat image, templateImage;
    QString category;

    QDir dataDirectory(dataDirName);

    foreach(QFileInfo info, dataDirectory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::AllDirs))
    {
        if(info.isDir())
        {
            category = info.baseName();
            std::string dir = info.absoluteFilePath().toStdString() +
                    "/" + category.toStdString() + ".jpg";
            categoryNames.append(category);
            image = cv::imread(dir.c_str(), 1);
            cv::cvtColor(image, templateImage, CV_BGR2GRAY);
            templates[category] = templateImage;

        }
    }
}


void generateTemplateFeatures(QWidget *parent, QString dataDirName, QList<QString> categoryNames,
                              QMap<QString, cv::Mat> templates,
                              QMap<QString, std::vector<cv::KeyPoint> > &keypoints,
                              QMap<QString, cv::Mat> &desc)
{
      QString descFile = dataDirName + "desc.xml";
      cv::FileStorage fs(descFile.toStdString(), cv::FileStorage::WRITE);

      if(templates.isEmpty())
      {
          QMessageBox::critical(parent, "Error", "Could not find template images");
      }
      else
      {

          int categories = categoryNames.size();


          for(int i = 0; i < categories; i++)
          {
              QString category = categoryNames[i];

              generateKpDesc(templates[category], keypoints[category], desc[category]);
          }
      }
      //serializeKeypoints(keypoints, dataDirName);
      fs.release();
}



void generateKpDesc(cv::Mat img, std::vector<cv::KeyPoint> &kp, cv::Mat &desc)
{
    cv::Ptr<cv::FeatureDetector> featureDetector;
    cv::Ptr<cv::DescriptorExtractor> descriptorExtractor;

    featureDetector = new cv::SurfFeatureDetector(500);
    descriptorExtractor = new cv::SurfDescriptorExtractor();

    featureDetector -> detect(img, kp);
    descriptorExtractor -> compute(img, kp, desc);

}


void addDirectory(QString dirName)
{
    QDir dir(dirName);
    if(!dir.exists())
    {
        QDir().mkdir(dirName);
    }
}

void detectChessboard(const cv::Mat &frame, cv::Size patternSize)
{
    cv::Mat gray; //source image
    cv::cvtColor(frame, gray, CV_BGR2GRAY);
    std::vector<cv::Point2f> corners; //this will be filled by the detected corners

    //CALIB_CB_FAST_CHECK saves a lot of time on images
    //that do not contain any chessboard corners
    bool patternFound = cv::findChessboardCorners(gray, patternSize, corners,
                                                  cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE +
                                                  cv::CALIB_CB_FAST_CHECK);

    if(patternFound)
    {
      cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                       cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
    }
    cv::drawChessboardCorners(frame, patternSize, cv::Mat(corners), patternFound);
}
