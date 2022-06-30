/////
/// Rev1.0  发行版本
/// Rev1.1  将数据进行封装，const重构函数
/// Rev1.2  添加数据处理类，对接收到的数据进行处理，保存，读取等
/// Rev1.3  修改LM的u数值为1.0e-4（原数值为1.0e-3);修改o和s算法




#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

}
