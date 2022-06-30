#ifndef OPTICONSOLE_H
#define OPTICONSOLE_H

#include <filmdesign2.h>
#include <Optimization.h>
#include <math.h>
#include <QDebug>
class OptiConsole
{
private:

    const double DERIV_STEP = 1e-5;


    FilmDesign2* TheoryDesign;
    FilmDesign2* SimulateDesign;

    Optimization Simulate;

    vector<double> vecOMSDataTime;
    vector<double> vecOMSDataT;

public:
    OptiConsole();
    ~OptiConsole();
    void LoadTheoryDesign(string fileName);     //导入理论设计
    void LoadTheoryDesignToSimualte();          //将TheoryDesign的类复制给SimulateDesign,通常用于切换层，将理论设计的信息传递给simulate进行模拟时
    void FreshSimulateDesign(const unsigned int &layer);                 //切换层时刷新SimulateDesign
    void InitiateSimulateDesign();                                       //初始化SimulateDesign

    void LoadOMSData(vector<double> vecTime,vector<double> vecT);   //导入测试数据进入类
    void LoadLayerInfo(const unsigned int &nCurLayer);                     //将当前层导入到类中

    void Fitting();       //模拟曲线


    void GetFittingNR(double &fittingN,double &fittingRate);               //获取拟合的折射率,速率
    vector<double> GetFittingT(double &X2);       //获取拟合的透过率,和对应的X2值

    void GetFittingInfo(double &QW_Phase,double &QW_Offset,double &QW_CutPeak,
                      double &Tend_CutPeak,double &Tend_Offset_theory,double &Tend_Offset_act,double &Tend_Phase_act,
                      double &Pend_theory,double &Phend_act,double &DeltaPhase,
                      double &Time_CutPeak,double &Time_Offset,double &Time_Phase,
                      double &fittingN,double &fittingRate,double &X2,vector<double> &SimulateT,vector<double> &test);              //获取不同算法的QW,透过率,相位,时间,相关数据


    double GetGradientOffset(const double &QW,const double &T_offset_Theory);
    double GetGradientPhase(const double &QW,const double &Phase_Theory);

    bool isEqualControlTypeForCurLayer(const string &controlType);


    //获取理论设计的相关信息
    int GetTheoryDesignCurLayer();      //获取理论设计的当前层
    string GetTheoryDesignDesignName(); //获取理论设计的名称
    bool GetTheoryDesignIsNoNull();     //判断理论设计是否存在
    int GetTheoryDesignTotalLayer();    //获取理论设计的总层数
    string GetTheoryDesignLayerControlType(const int &nLayerItem);  //获得对应层的理论设计的控制方式
    double GetTheoryDesignLayerQW_RefWL(const int &nLayerItem);
    double GetTheoryDesignLayerQW(const int &nLayerItem);
    double GetTheoryDesignLayerMonitorWL(const int &nLayerItem);
    string GetTheoryDesignLayerMaterial(const int &nLayerItem);     //获取nLayerItem层的理论设计的material
    double GetTheoryDesignLayerN(const int &nLayerItem);            //获取nLayerItem层的理论设计的N
    double GetTheoryDesignLayerRate(const int &nLayerItem);         //获取nLayerItem层的理论设计的Rate
    double GetTheoryDesignOMSWL();
    double GetTheoryDesignRefWL();      //获取理论设计的参考波长

    //设置理论设计的相关信息
    void SetTheoryDesignCurLayer(const int &nval);     //设置理论设计的当前层
    void SetTheoryDesignClearPreMatrix();              //重新计算PreMatrix
    void TheoryDesignTransRefQWtoMonitorQW(const int &nLayerItem);  //转换对应层的监控波长


    //获取模拟设计的相关信息
    bool GetSimulateDesignIsOutTol();


};


#endif // OPTICONSOLE_H
