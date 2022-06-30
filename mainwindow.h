#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

#include <vector>

#include <QFileDialog>
#include <QStandardItemModel>
#include <Optimization.h>
#include <opticonsole.h>

#include <QtCharts>
#include <QTcpSocket>

#include <QTimer>

#include <QCoreApplication>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QSettings>
#include <QUdpSocket>

#include "datasolution.h"


QT_CHARTS_USE_NAMESPACE
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT
private:

    QStandardItemModel *model;
    static OptiConsole *console;
    QChart* m_chart;
    QLineSeries* omsData;
    QLineSeries* simuData;

    static vector<double> omsDataTime;  //去除前10%个数据，用于拟合使用
    static vector<double> omsDataT;     //去除前10%个数据，用于拟合使用

    vector<double> omsTotalTime;        //完整的time数据，用于画图
    vector<double> omsTotalT;           //完整的T数据，用于画图

    static double dPreCrystal;
    static double dPreTime;

    std::mutex mu;       //用于锁定数据

    QTcpSocket *socket;
    bool isConnect;

    QTimer* GetDataTimer;   //用于模拟测试
    QTimer* NextLayerTimer; //用于5s钟倒计时，
    bool isNextLayer;       //用于倒计时
    bool isClickedNextLayer;//用于单个点击倒计时

    vector<double> vecLoadTime;     //该层导入的数据，用于判定是否有需要设定lastTime;
    vector<double> vecLoadCrystal;
    static double lastTime;             //用于层保存数据
    static double lastCrystal;          //用于层保存数据
    static vector<double> simuTime;     //用于层保存数据
    static vector<double> simuT;        //用于层保存数据，拟合后的数据

    double TargetCrystal = 0;
    double TargetTime = 0;

private slots:
//    void SimulateProcess();                  //模拟监控过程(用于Thread,后面没用了)
public:
    //获取CPUID等信息
    QString GetCPUID();
    QString GetBaseBoardSerial();
    QString GetDiskDriveSerial();
    QString GetMac();
    bool JudgeTime();    //判断时间
    bool JudgeMacAndCPUid();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Initiate();            //初始化画图选项
    void FittingFunc(unsigned int curLayer,vector<double> omsDataTime,vector<double> omsDataT);         //模拟数据

    void SolutionData(string strList,const int &flag);                                              //对输入的数据进行处理,flag==0为Log,flag==1为OTFC1300
    void ChangeLayerFresh(const int &layer);                                                       //切换层时刷新所有数据
    void LoadInfo(double &time,double &crystal);                                                   //将读取的数据处理后导入到vector中，判断是否为中断后继续镀膜的
    void IsControlByT(const int &nLayer,const double &crystal);                                    //判断是否控制方式为T
    void ReduceTotalData();                                                                         //将原始数据去除前10%数据后导入到omsDataTime和omsDataT
    bool readLayerInfo(double &crystal,double& time);                                              //读取LayerInfo//层开始阶段，读取历史数据，并确认本层的目标晶控值和时间
    void getCurLayerOMSInfo();                                                                     //切换层时需要判断是否已存在该层的数据，以及该层晶控值


signals:
    void SendSimulateCurve(vector<double>omsTime,vector<double>omsT);
    void SendSimulateInfo(QVector<double>ParaInfo);

private slots:

    void on_BtnLoadDesign_clicked();                //导入RCP文件
    void on_BtnStart_clicked();                     //开始\停止镀膜
    void on_Btn_Stop_clicked();                     //暂停\继续镀膜
    void on_Btn_NextLayer_clicked();                //切换下一层
    void on_Btn_SetLot_clicked();                   //设定LotNumber
    void on_BtnConnect_clicked();                   //连接TCPIP

    //发送命令
    void socket_Disconnected();                     //断开连接
    void socket_Stop();                             //发送停止信号
    void socket_Pause();                            //发送暂停信号
    void socket_Continue();                         //发送继续信号
    void socket_NextLayer();                        //发送切换层信号

    //主线程
    void socket_ReadData();                         //读取信号,主线程(用于读取数据,模拟数据等)
    void NextLayer();                               //倒计时到，发送切换命令

    //显示数据
    void ShowDesignInfo();                                            //显示膜层的信息
    void ShowSimuData(vector<double>simuTime,vector<double>simuT);   //显示模拟曲线
    void ShowSimuPara(QVector<double>simuInfo);                      //显示模拟数据
    void ShowStopTime(double deltaTime,QString controlType);         //倒计时的显示
    bool ShowOMSData(vector<double>omsTime,vector<double>omsT);      //显示原始数据
    void ShowRawData(const int &coatLayer,const string layerMat,const double &time,
                     const double &t,const double &crystal,const double &rate);//在标签上显示原始数据
    void ShowSimulateInfo(vector<double> omsDataTime);               //获取模拟数据并显示相关数据和曲线

    void ClearStopTime();                                            //清空倒计时的显示
    void ClearFittingInfo();                                         //清空FittingInfo信息
    void saveLogData(string strInfo);                                //总的Log过程信息保存

public:
    std::thread *threadFitting;         //Fitting线程
public:
    static bool isStarted;
    static bool isFitting;

    //模拟数据分析相关内容

private slots:
    void Debug();
    void on_BtnLoadLog_clicked();                   //设定Log数据
    void on_Btn_Simuate_clicked();
    void on_Btn_Test_clicked();

public:

    string GetLayerInfo(int nCount);        //调试——获取数据
    int GetLayerInfoCount();             //调试-获取数据文件中数据的个数
    int nInfoItem;          //数据所在位置

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
