#-------------------------------------------------
#
# Project created by QtCreator 2014-08-09T16:35:22
#
#-------------------------------------------------

QT       += core gui
QT       += webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenCVProject
TEMPLATE = app

DESTDIR = $$PWD

SOURCES += main.cpp\
        mainwindow.cpp \
    categorizerthread.cpp \
    dictionarydialog.cpp \
    selectionwidget.cpp \
    utilities.cpp \
    stereocameradialog.cpp \
    dictionarythread.cpp \
    stereocalibrationdialog.cpp \
    calibrationthread.cpp \
    disparitythread.cpp

HEADERS  += mainwindow.h \
    categorizerthread.h \
    dictionarydialog.h \
    selectionwidget.h \
    utilities.h \
    stereocameradialog.h \
    dictionarythread.h \
    stereocalibrationdialog.h \
    calibrationthread.h \
    disparitythread.h

FORMS    += mainwindow.ui \
    dictionarydialog.ui \
    stereocameradialog.ui \
    stereocalibrationdialog.ui

INCLUDEPATH += E:/opencv/build/include \
    E:/opencv/build/include/opencv \
    E:/opencv/build/include/opencv2 \

LIBS += -LE:/opencv/build/x86/vc11/lib/ \
    -lopencv_core246 \
    -lopencv_highgui246 \
    -lopencv_imgproc246 \
    -lopencv_features2d246 \
    -lopencv_calib3d246 \
    -lopencv_ml246 \
    -lopencv_nonfree246 \
    -lopencv_flann246 \

RESOURCES +=

