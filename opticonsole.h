#ifndef OPTICONSOLE_H
#define OPTICONSOLE_H

#include <filmdesign2.h>
#include <Optimization.h>
#include <math.h>
#include <QDebug>
class OptiConsole
{
public:

    const double DERIV_STEP = 1e-5;

    OptiConsole();
    ~OptiConsole();
    FilmDesign2* TheoryDesign;
    FilmDesign2* SimulateDesign;

    Optimization Simulate;

    vector<double> vecOMSDataTime;
    vector<double> vecOMSDataT;


    void LoadTheoryDesignToSimualte();          //将TheoryDesign的类复制给SimulateDesign,通常用于切换层，将理论设计的信息传递给simulate进行模拟时
    void LoadOMSData(vector<double> vecTime,vector<double> vecT);   //导入测试数据进入类
    void LoadLayerInfo(unsigned int nCurLayer);                     //将当前层导入到类中

    void Fitting();       //模拟曲线


    void GetFittingNR(double &fittingN,double &fittingRate);               //获取拟合的折射率,速率
    vector<double> GetFittingT(double &X2);       //获取拟合的透过率,和对应的X2值

    void GetFittingInfo(double &QW_Phase,double &QW_Offset,double &QW_CutPeak,
                      double &Tend_CutPeak,double &Tend_Offset_theory,double &Tend_Offset_act,double &Tend_Phase_act,
                      double &Pend_theory,double &Phend_act,double &DeltaPhase,
                      double &Time_CutPeak,double &Time_Offset,double &Time_Phase,
                      double &fittingN,double &fittingRate,double &X2,vector<double> &SimulateT,double &test);              //获取不同算法的QW,透过率,相位,时间,相关数据


    double GetGradientOffset(double QW,double T_offset_Theory);
    double GetGradientPhase(double QW,double Phase_Theory);


};


#endif // OPTICONSOLE_H
