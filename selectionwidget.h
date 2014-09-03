#ifndef SelectionWidget_H
#define SelectionWidget_H

#include <QLabel>

#include <opencv2/opencv.hpp>

class SelectionWidget : public QLabel
{
    Q_OBJECT

public:

    SelectionWidget(QWidget *parent = 0);
    ~SelectionWidget();

    void setObjectName(const QString &objectName);
    QString getObjectName() const;

    QRect getSelectionRect() const;

    void discardRect();
    cv::Mat save(const QString &dataFolder);

protected:

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:

    bool selectionStarted;
    QRect selectionRect;
    QString objectName;

signals:

    void mousePressed();

};

#endif // SelectionWidget_H
