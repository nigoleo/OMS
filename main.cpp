/////
/// Rev1.0  发行版本
/// Rev1.1  将数据进行封装，const重构函数





#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

}
