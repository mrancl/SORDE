#include "mainwindow.h"
#include <QApplication>

int handleError( int status, const char* func_name,
            const char* err_msg, const char* file_name,
            int line, void* userdata )
{
    //Do nothing -- will suppress console output
    return 0;   //Return value is not used
}

int main(int argc, char *argv[])
{
    cv::redirectError(handleError);

    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}
