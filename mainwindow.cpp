﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

bool MainWindow::isStarted = false;
bool MainWindow::isFitting = false;
OptiConsole* MainWindow::console = new OptiConsole();

vector<double> MainWindow::omsDataTime;
vector<double> MainWindow::omsDataT;
unsigned int MainWindow::nLayer = 0;
double MainWindow::dPreCrystal = 0;
double MainWindow::dPreTime = 0;

double MainWindow::lastTime = 0;             //用于层保存数据
double MainWindow::lastCrystal = 0;              //用于层保存数据
vector<double> MainWindow::simuTime;     //用于层保存数据
vector<double> MainWindow::simuT;        //用于层保存数据


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("OMS_Test_Rev1.0");

    //考虑添加获取北京时间功能选项_用于授权功能

    socket = new QTcpSocket();
    isConnect = false;  //用于开启读取数据线程
    //连接槽
    connect(socket,&QTcpSocket::readyRead,this,&MainWindow::socket_ReadData);
    connect(socket,&QTcpSocket::disconnected,this,&MainWindow::socket_Disconnected);

    connect(this,SIGNAL(SendSimulateCurve(vector<double>,vector<double>)),this,SLOT(ShowSimuData(vector<double>,vector<double>)));   //显示模拟曲线
    connect(this,SIGNAL(SendSimulateInfo(QVector<double>)),this,SLOT(ShowSimuPara(QVector<double>)));                                //显示模拟参数

    ui->lE_IP->setText("127.0.0.1");
    ui->lE_Port->setText("3306");

    GetDataTimer = new QTimer();
    NextLayerTimer = new QTimer();
    isNextLayer = false;        //是否需要切换下一层
    isClickedNextLayer = false;

    //设置显示参数的背景色
    QPalette pallete;
    pallete.setColor(QPalette::Background,Qt::white);
    ui->label_Layer->setAutoFillBackground(true);
    ui->label_Material->setAutoFillBackground(true);
    ui->label_Crystal->setAutoFillBackground(true);
    ui->label_Time->setAutoFillBackground(true);
    ui->label_T->setAutoFillBackground(true);
    ui->label_Rate->setAutoFillBackground(true);

    ui->label_Layer->setPalette(pallete);
    ui->label_Material->setPalette(pallete);
    ui->label_Crystal->setPalette(pallete);
    ui->label_Time->setPalette(pallete);
    ui->label_T->setPalette(pallete);
    ui->label_Rate->setPalette(pallete);

    ui->statusbar->setSizeGripEnabled(false);   //去掉状态栏右下角的三角
    ui->Btn_Stop->setEnabled(false);            //设置暂停键失能
    Initiate();

    TargetCrystal = 0;
    TargetTime = 0;

    vector<double>().swap(vecLoadTime) ;
    vector<double>().swap(vecLoadCrystal) ;
    vector<double>().swap(omsTotalTime) ;
    vector<double>().swap(omsTotalT) ;

}

MainWindow::~MainWindow()
{
    delete this->socket;
    delete this->model;
    delete this->console;
    delete this->m_chart;
    delete this->omsData;
    delete this->simuData;

    delete ui;
}

bool MainWindow::JudgeTime()
{
    QDate curTime;
    int authezaition = curTime.currentDate().year()*100+curTime.currentDate().month();
    if (authezaition >= 202212)
    {
        return false;
    }
    return true;
}



QString MainWindow::GetCPUID()
{
    QString cpuid = "";
    QProcess p(nullptr);
    p.start("wmic CPU get ProcessorID");
    p.waitForStarted();
    p.waitForFinished();
    cpuid = QString::fromLocal8Bit(p.readAllStandardOutput());
    cpuid = cpuid.remove("ProcessorId").trimmed();
    return cpuid;
}

QString MainWindow::GetBaseBoardSerial()
{
    QString serial = "";
    QProcess p(nullptr);
    p.start("wmic baseboard get serialnumber");
    p.waitForStarted();
    p.waitForFinished();
    serial = QString::fromLocal8Bit(p.readAllStandardOutput());
    serial = serial.remove("SerialNumber").trimmed();
    return serial;

    //L1HF7BG0039
}
QString MainWindow::GetDiskDriveSerial()
{
    QString serial = "";
    QProcess p(nullptr);
    p.start("wmic diskdrive get serialnumber");
    p.waitForStarted();
    p.waitForFinished();
    serial = QString::fromLocal8Bit(p.readAllStandardOutput());
    serial = serial.remove("SerialNumber").trimmed();
    return serial;
//AA000000000000000774
}

QString MainWindow::GetMac()
{
    QStringList mac_list;
    QString strMac;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i<ifaces.count();i++)
    {
        QNetworkInterface iface = ifaces.at(i);

        //过滤掉本地回环地址、没有开启的地址
        if (iface.flags().testFlag(QNetworkInterface::IsUp)&&iface.flags().testFlag(QNetworkInterface::IsRunning))
        {

            if(strMac.isEmpty()||strMac<iface.hardwareAddress())
            {
                strMac = iface.hardwareAddress();
            }
        }
    }
    return strMac;
    //60:14:B3:CB:75:5D
}


bool MainWindow::JudgeMacAndCPUid()
{
    QString cpu_id = "BFEBFBFF000406E3";            //本机的cpuID

    QString baseBoardId = "L1HF7BG0039";         //本机的mac地址
    if((cpu_id==GetCPUID())&&(baseBoardId == GetBaseBoardSerial()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::Initiate()
{

    m_chart= new QChart();
    omsData= new QLineSeries();

    simuData= new QLineSeries();

    m_chart->addSeries(omsData);
    m_chart->addSeries(simuData);

    ui->tV_Chart->setChart(m_chart);
    omsData->setColor(Qt::blue);
    simuData->setColor(Qt::red);
    m_chart->legend()->hide();
    m_chart->createDefaultAxes();

    GetDataTimer = new QTimer(this);
    connect(GetDataTimer,SIGNAL(timeout()),this,SLOT(Debug()));
    NextLayerTimer = new QTimer(this);
    connect(NextLayerTimer,SIGNAL(timeout()),this,SLOT(NextLayer()));
}

//连接TCP/IP
void MainWindow::on_BtnConnect_clicked()
{
    if(ui->BtnConnect->text() == tr("connect"))
    {
        QString IP;
        quint16 port;
        //获取IP地址
        IP = ui->lE_IP->text();
        port = ui->lE_Port->text().toUShort();

        //取消已有连接
        socket->abort();
        //连接服务器
        socket->connectToHost(IP,port);
        //等待连接成功
        if(!socket->waitForConnected(3000))
        {
            //qDebug()<<"Connect Failed";
            return;
        }
        //qDebug()<<"Connect successfully!";
        //发送按钮使能
        ui->BtnConnect->setText("DisConnect");
    }
    else
    {
        //断开连接
        socket->disconnectFromHost();
        //修改按钮文字
        ui->BtnConnect->setText("connect");
    }
}
//读取信号,进行数据处理和模拟信号
void MainWindow::socket_ReadData()
{
    QByteArray buffer;
    //读取缓冲区数据
    buffer = socket->readAll();
    if(!buffer.isEmpty())
    {
        if(isStarted)   //是否启动FittingFunc
        {
            QString str;
            str = tr(buffer);
            QStringList strList = str.split(",");
            SolutionData(strList);

            if(isFitting&&(console->TheoryDesign->GetLayerControlType(console->TheoryDesign->curLayer)!="T"))
            {
                FittingFunc(nLayer,omsDataTime,omsDataT);

                //如果模拟参数超出一定范围时，重新设定模拟设定值
                if (console->SimulateDesign->isOutTol())
                {
                    console->LoadTheoryDesignToSimualte();
                }
            }
            QThread::msleep(1);
        }
    }
}
//断开连接
void MainWindow::socket_Disconnected()
{
    //发送按钮失能
    ui->BtnConnect->setText("connect");
    //qDebug()<<"Disconnected";
}

void MainWindow::on_BtnStart_clicked()
{
    //启动主线程
    //需要确认Design和LotNum
    if (FileName == "")
    {
        QMessageBox::warning(this,"warning","Please set the lot number.");
        return;
    }
    if (!console->TheoryDesign->isNoNull)
    {
        QMessageBox::warning(this,"warning","Please check the Deisgn is loaded in.");
        return;
    }

    if(!isStarted)
    {
        isStarted = true;
        ui->BtnStart->setText("Stop");
        QPalette pallete;
        pallete.setColor(QPalette::Background,Qt::green);
        ui->BtnStart->setPalette(pallete);
        ui->Btn_Stop->setEnabled(true);
        saveLogData("Start Coating!");
        console->TheoryDesign->curLayer = 0;
        console->TheoryDesign->ClearPreMat();
        console->LoadTheoryDesignToSimualte();
        //需要刷新下历史数据（如果模拟数据过）
        vector<double>().swap(omsDataTime);
        vector<double>().swap(omsDataT);
    }
    else
    {
        isStarted = false;
        ui->BtnStart->setText("Start");
        QPalette pallete;
        pallete.setColor(QPalette::Background,Qt::gray);
        ui->BtnStart->setPalette(pallete);
        ui->Btn_Stop->setEnabled(false);
        socket_Stop();      //需要发送停止命令
    }
//    threadFitting = new std::thread(&MainWindow::SimulateProcess,this);
//    threadFitting->detach();
}

void MainWindow::on_Btn_Stop_clicked()
{
    if(!isStarted)
    {
        isStarted = true;
        ui->Btn_Stop->setText("Continue");
        QPalette pallete;
        pallete.setColor(QPalette::Background,Qt::green);
        ui->BtnStart->setPalette(pallete);
        socket_Continue();

        //需要刷新下历史数据（如果模拟数据过）
        vector<double>().swap(omsDataTime);
        vector<double>().swap(omsDataT);
        getCurLayerOMSInfo();
    }
    else
    {
        isStarted = false;
        ui->Btn_Stop->setText("Pause");
        QPalette pallete;
        pallete.setColor(QPalette::Background,Qt::gray);
        ui->BtnStart->setPalette(pallete);
        socket_Pause();             //需要发送暂停命令
    }
}

void MainWindow::FittingFunc(unsigned int curLayer,vector<double> omsDataTime,vector<double> omsDataT)
{
    try
    {
        QTime time;
        time.start();
        console->LoadOMSData(omsDataTime,omsDataT);
        //如果导入的层与理论设定层不一致，需要重新刷新PreMat
        if(curLayer != console->SimulateDesign->curLayer)
        {
            console->TheoryDesign->curLayer = curLayer;
            console->TheoryDesign->ClearPreMat();
            console->LoadTheoryDesignToSimualte();
        }
        console->Fitting();  
        ShowSimulateInfo(omsDataTime);          //显示相关的模拟信息

        int time_Diff = time.elapsed();
        ui->statusbar->showMessage(QString("Fitting:%1_ms").arg(time_Diff));
    }
    catch(exception ex)
    {

    }
}

void MainWindow::ShowSimulateInfo(vector<double> omsDataTime)
{
    double QW_Phase = 0;
    double QW_Offset = 0;
    double QW_CutPeak = 0;
    double Tend_CutPeak = 0;
    double Tend_Offset_theory = 0;
    double Tend_Offset_act = 0;
    double Tend_Phase_act = 0;

    double Pend_theory=0;
    double Phend_act=0;
    double DeltaPhase=0;


    double Time_CutPeak = 999;
    double Time_Offset = 999;
    double Time_Phase = 999;

    double fittingN = 0;
    double fittingRate =0;
    double X2 =0;
    vector<double> SimulateT;
    double testCutPeak = 0;
    console->GetFittingInfo(QW_Phase,QW_Offset,QW_CutPeak,
                          Tend_CutPeak,Tend_Offset_theory,Tend_Offset_act,Tend_Phase_act,
                          Pend_theory,Phend_act,DeltaPhase,
                          Time_CutPeak,Time_Offset,Time_Phase,
                          fittingN,fittingRate,X2,SimulateT,testCutPeak);
    QVector<double> info;
    info.append(QW_Phase);
    info.append(QW_Offset);
    info.append(QW_CutPeak);

    info.append(Tend_CutPeak);
    info.append(Tend_Offset_theory);
    info.append(Tend_Offset_act);
    info.append(Tend_Phase_act);

    info.append(Pend_theory);
    info.append(Phend_act);
    info.append(DeltaPhase);

    info.append(Time_CutPeak);
    info.append(Time_Offset);
    info.append(Time_Phase);

    info.append(fittingN);
    info.append(fittingRate);
    info.append(X2);

    info.append(testCutPeak);
    //显示模拟曲线
    //****************需要添加画图的功能**********************************//
    emit SendSimulateCurve(omsDataTime,SimulateT);

    //****************显示参数数据***************************************//
    emit SendSimulateInfo(info);

    //返回选定方式的倒计时***先不考虑
    double DeltaTime = 9999;
    if (console->TheoryDesign->GetLayerControlType(console->TheoryDesign->curLayer)=="O")
    {
        DeltaTime = Time_Offset - omsDataTime[omsDataTime.size()-1];
    }
    else if (console->TheoryDesign->GetLayerControlType(console->TheoryDesign->curLayer)=="S")
    {
        DeltaTime = Time_Phase - omsDataTime[omsDataTime.size()-1];
    }
    else
    {
        DeltaTime = Time_CutPeak - omsDataTime[omsDataTime.size()-1];                 //默认为CutPeak控制方式
    }
    if (DeltaTime<10)
    {
        ShowStopTime(DeltaTime,"OMS");
        saveLogData(QString("Layer_%1 will be stoped at %2").arg(console->TheoryDesign->curLayer).arg(DeltaTime));
    }
    if(DeltaTime<5)
    {
        vector<double>().swap(simuTime);
        vector<double>().swap(simuT);
        simuTime = omsDataTime;
        simuT = SimulateT;
        //发送倒计时计时器
        if (!isNextLayer)
        {
            NextLayerTimer->start(int(DeltaTime*1000)-500);
            isNextLayer = true;
        }
    }
}

void MainWindow::ShowStopTime(double deltaTime,QString controlType)
{
    if (controlType == "Crystal")
    {
        //不显示晶控差值
    }
    else
    {
        //显示deltaTime;
        ui->label_status->setText(QString::number(deltaTime,'f',1));
        QPalette pallete;
        pallete.setColor(QPalette::Background,Qt::yellow);
        ui->label_status->setAutoFillBackground(true);
        ui->label_status->setPalette(pallete);
    }
}

void MainWindow::ClearStopTime()
{
    QPalette pallete;
    pallete.setColor(QPalette::Background,QColor(231,231,231));
    ui->label_status->setAutoFillBackground(true);
    ui->label_status->setPalette(pallete);
    ui->label_status->clear();
}

void MainWindow::ClearFittingInfo()
{
    ui->label_FittingInfo->clear();
    ui->label_FittingInfo_2->clear();
}

void MainWindow::on_BtnLoadDesign_clicked()
{
/*    if (!JudgeTime())
    {
        this->close();
        QMessageBox::warning(this,"warning","Please Check warrant!");
        return;
    }
    if(!JudgeMacAndCPUid())
    {
        this->close();
        QMessageBox::warning(this,"warning","Please Check warrant!");
        return;
    }*/


    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "open",
                                                    "../",
                                                    "RCP(*.rcp)");
    console->TheoryDesign->GetFileInfo(filePath.toStdString());
    ShowDesignInfo();
    ui->label->setText(QString("Name:%1").arg(QString::fromStdString(console->TheoryDesign->designName)));
    ui->label_3->setText(QString("Layer:%1").arg(QString::number(console->TheoryDesign->totalLayer)));
}

void MainWindow::on_Btn_SetLot_clicked()
{
    FileName = ui->lE_LotNum->text();
    FileAddress = QCoreApplication::applicationDirPath();
    FileAddress = FileAddress+"//"+FileName;

    QDir dir;
    if (!dir.exists(FileAddress))
    {
        dir.mkpath(FileAddress);
    }
    saveLogData("It has set a lot number.");
}

void MainWindow::on_Btn_NextLayer_clicked()
{
    socket_NextLayer();
    isFitting = false;
}

void MainWindow::ShowDesignInfo()
{
    model = new QStandardItemModel(this);
    model->setColumnCount(7);
    model->setHeaderData(0,Qt::Horizontal,QString("Layer"));
    model->setHeaderData(1,Qt::Horizontal,QString("Material"));
    model->setHeaderData(2,Qt::Horizontal,QString("RefQW"));
    model->setHeaderData(3,Qt::Horizontal,QString("RefWL"));
    model->setHeaderData(4,Qt::Horizontal,QString("QW"));
    model->setHeaderData(5,Qt::Horizontal,QString("MWL"));
    model->setHeaderData(6,Qt::Horizontal,QString("Control"));

    ui->tV_Design->setModel(model);

    ui->tV_Design->setColumnWidth(0,40);
    ui->tV_Design->setColumnWidth(1,50);
    ui->tV_Design->setColumnWidth(2,60);
    ui->tV_Design->setColumnWidth(3,60);
    ui->tV_Design->setColumnWidth(4,60);
    ui->tV_Design->setColumnWidth(5,60);
    ui->tV_Design->setColumnWidth(6,40);
    ui->tV_Design->verticalHeader()->hide();

    for (int i = 0 ;i<console->TheoryDesign->totalLayer;i++)
    {
        model->insertRow(i);
        model->setItem(i,0,new QStandardItem(QString::number(i+1)));
        model->setItem(i,1,new QStandardItem(QString::fromStdString(console->TheoryDesign->layerMat[i])));
        model->setItem(i,2,new QStandardItem(QString::number(console->TheoryDesign->layerQW_RefWL[i])));
        model->setItem(i,3,new QStandardItem(QString::number(console->TheoryDesign->refWL)));
        model->setItem(i,4,new QStandardItem(QString::number(console->TheoryDesign->layerQW[i])));
        model->setItem(i,5,new QStandardItem(QString::number(console->TheoryDesign->layerMonitorWL[i])));
        model->setItem(i,6,new QStandardItem(QString::fromStdString(console->TheoryDesign->layerControlType[i])));

        if ((i+1) < console->TheoryDesign->curLayer)
        {
            model->item(i,0)->setBackground(QBrush(Qt::green));
            model->item(i,1)->setBackground(QBrush(Qt::green));
            model->item(i,2)->setBackground(QBrush(Qt::green));
            model->item(i,3)->setBackground(QBrush(Qt::green));
            model->item(i,4)->setBackground(QBrush(Qt::green));
            model->item(i,5)->setBackground(QBrush(Qt::green));
            model->item(i,6)->setBackground(QBrush(Qt::green));
        }
        else if ((i+1) == console->TheoryDesign->curLayer)
        {
            model->item(i,0)->setBackground(QBrush(Qt::yellow));
            model->item(i,1)->setBackground(QBrush(Qt::yellow));
            model->item(i,2)->setBackground(QBrush(Qt::yellow));
            model->item(i,3)->setBackground(QBrush(Qt::yellow));
            model->item(i,4)->setBackground(QBrush(Qt::yellow));
            model->item(i,5)->setBackground(QBrush(Qt::yellow));
            model->item(i,6)->setBackground(QBrush(Qt::yellow));
        }
    }

}

bool MainWindow::ShowOMSData(vector<double>omsTime,vector<double>omsT)
{
    if (omsTime.size()!=omsT.size())
    {
        return false;
    }
    omsData->clear();
    m_chart->removeSeries(omsData);
    for(unsigned int i = 0; i<omsTime.size();i++)
    {
        omsData->append(omsTime[i],omsT[i]);
    }
    m_chart->addSeries(omsData);
    m_chart->createDefaultAxes();
    return true;
}

void MainWindow::ShowSimuData(vector<double>simuTime,vector<double>simuT)
{
    if (simuTime.size()!=simuT.size())
    {
        return ;
    }
    simuData->clear();
    m_chart->removeSeries(simuData);
    for(unsigned int i = 0; i<simuTime.size();i++)
    {
        simuData->append(simuTime[i],simuT[i]);
    }
    m_chart->addSeries(simuData);
    m_chart->createDefaultAxes();
    return ;
}

void MainWindow::ShowSimuPara(QVector <double>info)
{
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::blue);
    ui->label_FittingInfo->setPalette(pe);
    ui->label_FittingInfo->setText(QString("FittingN:%1     FittingRate:%2\n"
                               "X2:%3")
                       .arg(QString::number(info[13],'f',3)).arg(QString::number(info[14],'f',3)).arg(QString::number(info[15],'e',3)));
    ui->label_FittingInfo_2->setText(QString("CutPeak_TimeStop:%1    Tstop:%2 \n"
                            "Offset_TimeStop:%3    Tstop_theory:%4    Tstop_act:%5     \n"
                            "Phase_TimeStop:%6     PhaseStop:%7    DeltaPhase:%8")
                       .arg(QString::number(info[10],'f',3)).arg(QString::number(info[3],'f',3))
                       .arg(QString::number(info[11],'f',3)).arg(QString::number(info[4],'f',3)).arg(QString::number(info[5],'f',3))
                       .arg(QString::number(info[12],'f',3)).arg(QString::number(info[8],'f',3)).arg(QString::number(info[9],'f',3)));
//    ui->label_status->setText(QString("Tcutpeak:%1").arg(QString::number(info[16],'f',3)));
}

/*************************FittingThread*******************************
 * FittingThread()
 * {
 *      如果导入的数据个数与FittingThreadDataCount一致，那么不进行以下Func;
 *          fittingFunc();  根据不同的监控方式发送本层需要的镀膜时间,保存模拟数据信息
 *          根据镀膜时间和当前时间设定,<10s显示倒计时和启动切换层倒计时命令、关闭FittingThread、WriteLog、保存切换层时的模拟曲线和监控曲线
 * }
 * ***********************FittingThread********************************/

/**************************主线程SolutionData*************************
  MainThread()
  {
    获取数据；(保存数据-Func)
    根据数据进行分析，如果镀膜层发生更换，
        1）读取并判定本层预计的Crystal值，console中设计需要刷新模拟数据、WriteLog
        2）确认是否存在新一层的历史数据，并进行导入
    判定是否为Crystal控制时间
    Yes:等待Crystal达到设定目标值
    No:判定是否（镀膜时间已达到1/2理论时间）&&（Crystal控制模式）
        1）达到的话，开启FittingThread、WriteLog;
  }
***************************主线程END**********************************/

void MainWindow::SolutionData(QStringList strList)
{
    if (strList.size()<14 )
    {
        return;
    }
    unsigned int coatLayer=strList[0].toUInt();
    QString layerMat=strList[1];
    double time=strList[2].toDouble();
    double t=strList[9].toDouble();
    double crystal=strList[12].toDouble();
    double rate=strList[13].toDouble();

    ui->label_Layer->setText(QString::number(coatLayer));
    ui->label_Material->setText(layerMat);
    ui->label_Time->setText(QString::number(time,'f',3));
    ui->label_T->setText(QString::number(t,'f',3));
    ui->label_Crystal->setText(QString::number(crystal*100,'f',2));
    ui->label_Rate->setText(QString::number(rate,'f',2));
    nLayer = coatLayer;

    if (coatLayer!= console->TheoryDesign->curLayer)   //切换层数时
    {
        TargetCrystal = 0;
        TargetTime = 0;

        dPreTime = 0;
        dPreCrystal = 0;


        vector<double>().swap(vecLoadTime);
        vector<double>().swap(vecLoadCrystal);

        isFitting = false;
        m_chart->removeSeries(simuData);

        console->TheoryDesign->curLayer = coatLayer;
        console->TheoryDesign->TransRefQWtoMonitorQW(coatLayer);        //MWL修改为当前层所使用的监控波长
        ShowDesignInfo();

        console->TheoryDesign->ClearPreMat();
        vector<double>().swap(omsDataTime);
        vector<double>().swap(omsDataT);
        vector<double>().swap(omsTotalTime) ;
        vector<double>().swap(omsTotalT) ;

        lastCrystal = 0;
        lastTime = 0;

        readLayerInfo(TargetCrystal,TargetTime);            //判定当前层的目标时间和目标晶控值
        saveLogData(QString("Layer_%1 Begin!").arg(coatLayer)); //开始镀膜
        ClearStopTime();                                    //停止时间刷新
        ClearFittingInfo();

        isClickedNextLayer = false;                         //是否点击NextLayer

        //需要确认下读取是否存在本层OMS；如果已存在OMS数据，又重新开始这一层的镀膜，在这基础上继续添加数据（用于异常处理时出现的情况）????好像有Bug存在，stop再start就会退出

        getCurLayerOMSInfo();
    }
    //用于判定输入的数据重新归零，time归零
    vecLoadTime.push_back(time);
    vecLoadCrystal.push_back(crystal);
    if (vecLoadTime.size()>2)
    {
        if (time < vecLoadTime[vecLoadTime.size()-2])
        {
            dPreTime += vecLoadTime[vecLoadTime.size()-2];
            dPreCrystal += vecLoadCrystal[vecLoadCrystal.size()-2];
        }
    }
    //设定去除前10%数据的个数起点nStartPos
    int nStartPos = 20;
    for (int i = 0; i< vecLoadTime.size();i++)
    {
        if (vecLoadTime[i]>=TargetTime/10)
        {
            nStartPos = i;
            break;
        }
    }

    omsTotalTime.push_back(time+dPreTime);
    omsTotalT.push_back(t);

    ReduceTotalData(nStartPos);          //去除20个数据

    this_thread::sleep_for(chrono::microseconds(1));

    ShowOMSData(omsTotalTime,omsTotalT);          //显示oms数据曲线

    //strList需要确认下晶控值和时间，并进行修改,如果之前Crystal数据已经存在
    if(dPreCrystal > 0)
    {
        strList[12] = QString::number(strList[12].toDouble()+dPreCrystal);
    }
    QString str = strList[0];
    for (int i = 1 ; i< strList.size();i++)
    {
        str += ","+strList[i];
    }
    //str[2] time
    //str[12] crystal
    saveOMSData(coatLayer,str);       //保存oms数据

    lastTime = time+dPreTime;
    lastCrystal = crystal+dPreCrystal;
    lastCrystal = lastCrystal*100;

    //不同控制方式,如果为T的话，根据晶控值达到目标值停止
    if (console->TheoryDesign->GetLayerControlType(coatLayer)=="T")
    {
        double delta = TargetCrystal- lastCrystal;
        if (delta<10)
        {
            ShowStopTime(delta,"Crystal");
        }
        if (delta<1)
        {
            if (!isClickedNextLayer)
            {
                NextLayer();  //发送切换层命令；
                isClickedNextLayer = true;
            }
        }
        return;
    }
    else if (omsDataTime.size()>0)
    {
        if ((omsDataTime[omsDataTime.size()-1]>TargetTime/2)&&(!isFitting))
        {
            isFitting = true;
            saveLogData(QString("Layer_%1 Start FittingThread!").arg(coatLayer));
        }
    }

    //不同控制方式,如果不为T    //需要获取预计膜层时间的1/2，在一定时间后isFitting切换成true

}

void MainWindow::ReduceTotalData(unsigned int startPos)
{
    vector<double>().swap(omsDataTime);
    vector<double>().swap(omsDataT);

    if (omsTotalTime.size()>startPos)
    {
        for (unsigned int i = startPos; i<omsTotalTime.size();i++)
        {
            omsDataTime.push_back(omsTotalTime[i]);
            omsDataT.push_back(omsTotalT[i]);
        }
    }
}

void MainWindow::NextLayer()
{
    socket_NextLayer();
    saveLogData(QString("Stop Fitting!"));
    saveLayerInfo(console->TheoryDesign->curLayer,lastCrystal,lastTime);
    if (isFitting&(!isClickedNextLayer))
    {
        saveSimuData(console->TheoryDesign->curLayer,simuTime,simuT);
        NextLayerTimer->stop();
        isClickedNextLayer = true;
    }
    isFitting = false;
    isNextLayer = false;
}

/**********************发送命令*************************************/

void MainWindow::socket_Stop()
{
    socket->write(QString("Stop").toLatin1());    //ASCII码形式发送
    socket->flush();
    saveLogData("Stop");
    //
}
void MainWindow::socket_Pause()
{
    socket->write(QString("Pause").toLatin1());    //ASCII码形式发送
    socket->flush();
    saveLogData("Pause");
}
void MainWindow::socket_Continue()
{
    socket->write(QString("Continue").toLatin1());    //ASCII码形式发送
    socket->flush();
    saveLogData("Continue");
}
void MainWindow::socket_NextLayer()
{

    socket->write(QString("NextLayer").toLatin1());    //ASCII码形式发送
    socket->flush();
    saveLogData(QString("Layer_%1 End!").arg(console->TheoryDesign->curLayer));
}

/*************************End**************************************/

/*********************保存数据***********************************/

void MainWindow::saveLogData(QString strInfo)
{
    QString logFileName = FileAddress + "//" + FileName + "_Log.csv";
    QFile fileInfo(logFileName);

    strInfo = QDateTime::currentDateTime().toString("MM-dd hh:mm:ss") + "  " +strInfo;

    fileInfo.open( QIODevice::ReadWrite | QIODevice::Text |QIODevice::Append);
    QTextStream txtOutput(&fileInfo);
    txtOutput<<strInfo<<endl;
    fileInfo.close();

    QString str = ui->pTE_Process->toPlainText();
    str = strInfo +"\n" + str;
    ui->pTE_Process->setPlainText(str);
}

void MainWindow::saveOMSData(unsigned int coatLayer,QString strInfo)
{
    QString logFileName = FileAddress + "//Layer_" + QString::number(coatLayer) + "_omsData.csv";
    QFile fileInfo(logFileName);
    fileInfo.open( QIODevice::ReadWrite | QIODevice::Text |QIODevice::Append);
    QTextStream txtOutput(&fileInfo);

    txtOutput<<strInfo;
    fileInfo.close();
}

void MainWindow::saveSimuData(unsigned int coatLayer,vector<double>simuTime,vector<double>simuT)
{
    QString logFileName = FileAddress + "//Layer_" + QString::number(coatLayer) + "_simuData.csv";
    QFile fileInfo(logFileName);
    fileInfo.open( QIODevice::ReadWrite | QIODevice::Text);
    QTextStream txtOutput(&fileInfo);
    try
    {
        for (unsigned int i = 0 ;i<simuTime.size();i++)
        {
            txtOutput<<coatLayer<<","<<simuTime[i]<<","<<simuT[i]<<endl;
        }
    }
    catch(exception ex)
    {
        qDebug()<<coatLayer<<"saveSimulateDataException"<<ex.what();
    }
    fileInfo.close();
}

void MainWindow::saveLayerInfo(unsigned int coatLayer,double crystal,double time)
{
    QString logFileName = FileAddress + "//" + FileName + "_TotalInfo.csv";
    QFile fileInfo(logFileName);
    fileInfo.open( QIODevice::ReadWrite | QIODevice::Text|QIODevice::Append);
    QTextStream txtOutput(&fileInfo);

    txtOutput<<coatLayer<<","<<crystal<<","<<time<<endl;

    fileInfo.close();
}

bool MainWindow::readLayerInfo(double &crystal,double& time)
{
    double QW = console->TheoryDesign->GetLayerQW(console->TheoryDesign->curLayer);
    double N = console->TheoryDesign->GetLayerN(console->TheoryDesign->curLayer);
    double Rate = console->TheoryDesign->GetLayerRate(console->TheoryDesign->curLayer);

    crystal = QW/4/N*console->TheoryDesign->OMSWL;               //理论晶控

    time = crystal/Rate*10;       //理论时间
    ui->label_TargetCrstal->setText(QString::number(crystal,'f',2));
    ui->label_TargetTime->setText(QString::number(time,'f',2));
    QString logFileName = FileAddress + "//" + FileName + "_TotalInfo.csv";
    QFile f(logFileName);

    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream txtInput(&f);
    QString lineStr;
    QStringList strList;
    vector<int> layerNo;
    vector<double> vecCrystal;
    vector<double> vecTime;
    while(!txtInput.atEnd())
    {
        try {
            lineStr = txtInput.readLine();
            strList = lineStr.split(",");
            layerNo.push_back(strList[0].toInt());
            vecCrystal.push_back(strList[1].toDouble());
            vecTime.push_back(strList[2].toDouble());
        } catch (exception ex) {
            return false;
        }
    }

    //进行数据对比处理
    double indexCrystal = 0;
    double indexTime = 0;
    int nCount = 0;

    for(int i = layerNo.size()-1;i>-1;i--)
    {
        int j = layerNo[i];
        if (console->TheoryDesign->layerMat[j-1] == console->TheoryDesign->layerMat[console->TheoryDesign->curLayer - 1])
        {
            double layerQW = console->TheoryDesign->layerQW[j-1];
            if (layerQW > 0)
            {
                indexCrystal += vecCrystal[i]/layerQW;
                indexTime += vecTime[i]/layerQW;
                nCount++;
            }
        }
        if (nCount>=3)
        {
            break;
        }
    }
    if (nCount != 0)
    {
        crystal = indexCrystal/(nCount)*console->TheoryDesign->GetLayerQW(console->TheoryDesign->curLayer);
        time = indexTime/(nCount)*console->TheoryDesign->GetLayerQW(console->TheoryDesign->curLayer);
        ui->label_TargetCrstal->setText(QString::number(crystal,'f',2));
        ui->label_TargetTime->setText(QString::number(time,'f',2));
    }
    return true;
}

void MainWindow::getCurLayerOMSInfo()
{
    QString logFileName = FileAddress + "//Layer_" + QString::number(console->TheoryDesign->curLayer) + "_omsData.csv";
    QFile fileInfo(logFileName);
    if(!fileInfo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ;
    }
    vector<double>().swap(vecLoadTime);
    vector<double>().swap(vecLoadCrystal);
    vector<double>().swap(omsTotalTime) ;
    vector<double>().swap(omsTotalT) ;

    QTextStream txtInput(&fileInfo);
    QString lineStr;
    QStringList strList;
    double preTime = 0;
    double preCrystal = 0;
    while(!txtInput.atEnd())
    {
        try {
            lineStr = txtInput.readLine();
            strList = lineStr.split(",");
            vecLoadTime.push_back(strList[2].toDouble());
            vecLoadCrystal.push_back(strList[12].toDouble());
            if (vecLoadTime.size()>2)
            {
                if (strList[2].toDouble() < vecLoadTime[vecLoadTime.size()-2])
                {
                    preTime += vecLoadTime[vecLoadTime.size()-2];
                    preCrystal += vecLoadCrystal[vecLoadCrystal.size()-2];
                }
            }

            omsTotalTime.push_back(strList[2].toDouble()+preTime);
            omsTotalT.push_back(strList[9].toDouble());
        } catch (exception ex) {
            return ;
        }
    }
    fileInfo.close();
}

/**********************End*********************************/




/*************************调试软件*************************************/

void MainWindow::on_BtnLoadLog_clicked()
{
//设定Log文件所在文件夹地址
    QString file_path = QFileDialog::getExistingDirectory(this, "请选择文件夹路径...", "./");

    if(file_path.isEmpty())

    { return; }

    ui->lE_FileAddress->setText(file_path);
}

void MainWindow::on_Btn_Simuate_clicked()
{
    //开启模拟数据
    if (isStarted)
    {
        QMessageBox::warning(this,"warning","It is coating now.Please check the state.");
        return ;
    }
    nInfoItem = 0;
    if (!GetDataTimer->isActive())
    {
        GetDataTimer->start(300);
        ui->Btn_Simuate->setText("Simulating");
    }
    else
    {
        GetDataTimer->stop();
        ui->Btn_Simuate->setText("Start_Simulate");
    }
}

QStringList MainWindow::GetLayerInfo(int nCount)
{
    QStringList strlist;
    QString FileAddress = ui->lE_FileAddress->text();
    int LayerInfo = ui->lE_FileLayer->text().toInt();
    QString FileName = FileAddress;
    FileName += QString("\\LAYER_%1.csv").arg(LayerInfo);
    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug()<<"Can't open the file!"<<endl;
    }
    for(int i = 0; i<nCount;i++)
    {
        QByteArray line = file.readLine();
        QString str(line);
        strlist = str.split(",");
    }
    return strlist;
}

int MainWindow::GetLayerInfoCount()
{
    QString FileAddress = ui->lE_FileAddress->text();
    int LayerInfo = ui->lE_FileLayer->text().toInt();
    QString FileName = FileAddress;
    FileName += QString("\\LAYER_%1.csv").arg(LayerInfo);
    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug()<<"Can't open the file!"<<endl;
    }
    int nCount = 0;
    while(!file.atEnd())
    {
        file.readLine();
        nCount++;
    }
    return nCount;
}

void MainWindow::Debug()
{
    int Count = GetLayerInfoCount();
    QStringList x = GetLayerInfo(nInfoItem);
    QThread::msleep(1);
    SolutionData(x);
    FittingFunc(ui->lE_FileLayer->text().toUInt(),omsDataTime,omsDataT);
    nInfoItem ++;
    if(nInfoItem == Count)
    {
        GetDataTimer->stop();
    }

}

/****************************End*******************************/


void MainWindow::on_Btn_Test_clicked()
{
    /*
    saveLayerInfo(1,539.42,2.1484001);
    saveLayerInfo(2,387.303,2.9603);
    saveLayerInfo(3,539.545,2.1461);
    double crystal = 0;
    double time = 0;
    console->TheoryDesign->curLayer = 5;
    readLayerInfo(crystal,time);
    qDebug()<<crystal<<time;
*/
    qDebug()<<console->TheoryDesign->curLayer;
    console->TheoryDesign->curLayer += 4;
    console->TheoryDesign->TransRefQWtoMonitorQW(console->TheoryDesign->curLayer);        //MWL修改为当前层所使用的监控波长
    ShowDesignInfo();

}
