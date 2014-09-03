#include "selectionwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDir>

#include <QDebug>

SelectionWidget::SelectionWidget(QWidget *parent)
    : QLabel(parent)
{
    selectionStarted=false;
}

SelectionWidget::~SelectionWidget()
{

}

void SelectionWidget::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    QPainter painter(this);
    painter.setPen(QPen(QBrush(QColor(0,0,0,180)),1,Qt::DashLine));
    painter.setBrush(QBrush(QColor(255,255,255,120)));

    painter.drawRect(selectionRect);
}

void SelectionWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton)
    {
        selectionStarted=true;
        selectionRect.setTopLeft(e->pos());
        selectionRect.setBottomRight(e->pos());
        emit mousePressed();
    }
}

void SelectionWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (selectionStarted)
    {
        selectionRect.setBottomRight(e->pos());
        repaint();
    }
}

void SelectionWidget::mouseReleaseEvent(QMouseEvent *e)
{
    selectionStarted=false;
}

void SelectionWidget::discardRect()
{
    if(selectionRect.isValid())
    {
        selectionRect.setSize(QSize(0, 0));
        repaint();
    }
}

cv::Mat SelectionWidget::save(const QString &dataFolder)
{

    QString dataDirName = dataFolder;

    dataDirName +=  objectName;
    QDir objectDir(dataDirName);
    if(!objectDir.exists())
    {
        QDir().mkdir(dataDirName);
    }

    QString fileName = dataDirName + "/" + objectName + ".jpg";

    this->pixmap()->copy(selectionRect).save(fileName);

    cv::Mat img;
    img = cv::imread(fileName.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
    return img;

}

void SelectionWidget::setObjectName(const QString &objectName)
{
    this->objectName = objectName;
}

QString SelectionWidget::getObjectName() const
{
    return objectName;
}

QRect SelectionWidget::getSelectionRect() const
{
    return selectionRect;
}

