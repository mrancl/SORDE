#include "dictionarythread.h"

#include <QDebug>

DictionaryThread::DictionaryThread(QString dataDir,
                                   QMap<QString, cv::Mat> templates,
                                   QMap<QString, cv::Mat> desc,
                                   QList<QString> categoryNames,
                                   int clusters)
{
    this->dataDir = dataDir;
    this->templates = templates;
    this->categoryNames = categoryNames;
    this->desc = desc;

    featureDetector = new cv::SurfFeatureDetector(500);
    descriptorExtractor = new cv::SurfDescriptorExtractor();
    bowtrainer = new cv::BOWKMeansTrainer(clusters);
    descriptorMatcher = new cv::FlannBasedMatcher();
    bowDescriptorExtractor = new cv::BOWImgDescriptorExtractor(descriptorExtractor, descriptorMatcher);

    progressCounter = 1;

    doStop = false;
}

DictionaryThread::~DictionaryThread()
{
    delete featureDetector;
    delete descriptorExtractor;
    delete bowtrainer;
    delete descriptorExtractor;
    delete bowDescriptorExtractor;
}

void DictionaryThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop = true;
}

void DictionaryThread::run()
{
    doStopMutex.lock();
    if(doStop)
    {
        doStop = false;
        doStopMutex.unlock();
    }
    doStopMutex.unlock();

    processingMutex.lock();

    makeTrainSet();
    buildVocab();
    trainClassifiers();

    processingMutex.unlock();

    // Inform GUI thread of new vocab and SVM
    emit doneGeneratingDictionary(svms, vocab);
}

void DictionaryThread::makeTrainSet()
{
    QString category;

    QDir dataDirectory(dataDir);
    dataDirectory.setFilter(QDir::Files | QDir::Dirs |
                           QDir::NoDot | QDir::NoDotDot);
    QDirIterator it(dataDirectory, QDirIterator::Subdirectories);

    while(it.hasNext()) {
        it.next();
        if(it.fileInfo().isDir() && !it.filePath().contains("Training_Images"))
        {
            category = it.fileName();
        }
        if(it.filePath().contains("Training_Images") && it.fileInfo().isFile())
        {

            trainSet.insert(category, it.fileInfo().absoluteFilePath());
        }

    }
    progressCounter = 5;
    emit updateProgress(progressCounter);
}

void DictionaryThread::makePosNeg()
{
    bowDescriptorExtractor -> setVocabulary(vocab);
    // Iterate through the whole training set of images
    QMultiMap<QString, QString>::iterator i;
    for (i = trainSet.begin(); i != trainSet.end(); i++)
    {
        QString category = i.key();
        cv::Mat feat, dsc;

        cv::Mat img, img_g;
        img = cv::imread(i.value().toStdString());
        cv::cvtColor(img, img_g, CV_BGR2GRAY);

        // Detect keypoints, get the image BOW descriptor
        std::vector<cv::KeyPoint> kp;
        featureDetector -> detect(img_g, kp);
        descriptorExtractor ->compute(img_g, kp,dsc);
        bowDescriptorExtractor -> compute(img_g, kp, feat);

        int categories = categoryNames.size();
        for(int cat_index = 0; cat_index < categories; cat_index++)
        {
            QString checkCategory = categoryNames[cat_index];
            // Add BOW feature as positive sample for current category ...
            if(checkCategory.compare(category) == 0)
            {
                positiveData[checkCategory].push_back(feat);
            }
            //... and negative sample for all other categories
            else
            {
                negativeData[checkCategory].push_back(feat);
            }
        }
    }
    progressCounter = 75;
    emit updateProgress(progressCounter);
}

void DictionaryThread::buildVocab()
{
    cv::Mat vocabDescriptors;
    QMap<QString, cv::Mat>::iterator i;
    for(i = desc.begin(); i != desc.end(); i++)
    {
        vocabDescriptors.push_back(i.value());
    }
    // Add the descriptors to the BOW trainer to cluster
    bowtrainer -> add(vocabDescriptors);
    // cluster the SURF descriptors
    vocab = bowtrainer -> cluster();

    // Save the vocabulary
    QString vocabFileName = dataDir + "vocab.xml";
    cv::FileStorage fs(vocabFileName.toStdString(), cv::FileStorage::WRITE);
    fs << "vocabulary" << vocab;
    fs.release();

    progressCounter = 50;
    emit updateProgress(progressCounter);

}

void DictionaryThread::trainClassifiers()
{
    // Extract BOW descriptors for all training images and organize them into positive and negative samples for each category
    makePosNeg();

    int categories = categoryNames.size();
    for(int i = 0; i < categories; i++)
    {
        QString category = categoryNames[i];
        //Positive training data has labels 1
        cv::Mat trainData = positiveData[category];
        cv::Mat trainLabels = cv::Mat::ones(trainData.rows, 1, CV_32S);

        //Negative training data has labels 0
        trainData.push_back(negativeData[category]);
        cv::Mat m = cv::Mat::zeros(negativeData[category].rows, 1, CV_32S);
        trainLabels.push_back(m);

        // SVM params
        cv::SVMParams params;
        params.kernel_type = cv::SVM::RBF;
        params.svm_type = cv::SVM::C_SVC;
        params.gamma = 0.50625000000000009;
        params.C = 312.50000000000000;
        params.term_crit = cv::TermCriteria(CV_TERMCRIT_ITER, 100, 0.000001);

        // Train SVM
        svms[category].train(trainData, trainLabels, cv::Mat(), cv::Mat(), params);

        // Save SVM to file for reuse
        QString svmFileName = dataDir + category + "SVM.xml";
        svms[category].save(svmFileName.toStdString().c_str());

        progressCounter = 100;
        emit updateProgress(progressCounter);
    }
}



