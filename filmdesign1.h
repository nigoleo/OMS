#ifndef FILMDESIGN1_H
#define FILMDESIGN1_H


/**************FilmDesign**********************
//算法：考虑indexT，N,Rate,_CurLayer,前三层的QW为模拟数据 ，前2个N为FittingN
**********************************************/


#include <string>
#include <vector>
#include <fstream>
#include "math.h"
#include <coatmatrix.h>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

using namespace std;

class FilmDesign1
{
public:
    FilmDesign1();
    const double DERVI_STEP = 0.00001;
    const double PenaltyLambda = 0.01;  //惩罚函数的系数
    double PI = 3.14159265;
    string designName;

    bool isNoNull;
    string HName;
    string LName;
    double HN;
    double LN;
    double HRate;
    double LRate;

    double HN_Original;     //用于惩罚函数
    double LN_Original;     //用于惩罚函数
    double HRate_Orignal;
    double LRate_Orignal;
    double subStrateN_Orignal;
    double _curQW_Orignal;
    const double deltaNTol = 0.3; //折射率容差为0.3

    double indexT;
    string subStrateName;
    double subStrateN;
    int totalLayer;
    unsigned int curLayer;
    double OMSWL;

    CoatMatrix preTotalMatrix;
    CoatMatrix preNoOptiMatrix;

    vector<double> checkParams;     //参数

    double _curQW; //预估当前层已完成的光学厚度
    bool isBackSideNull;  //背面是否存在
    vector<string> layerMat;
    vector<double> layerQW;
    vector<string> layerControlType;


    //添加多波长监控的方式
    vector<double> layerMonitorWL;
    vector<double> layerQW_RefWL;
    double refWL;

    void TransRefQWtoMonitorQW(unsigned int layerItem);

    bool GetFileInfo(string filePath);
    bool GetFileInfo1(string filePath);    //rev1的rcp导入
    bool GetFileInfo2(string filePath);    //rev2的rcp导入

    vector<string> Split(string str,string pattern);               //分割string到对应的vector<string>里面
    vector<double> InitiateParameter();                            //初始化parameters
    vector<double> GetParamter(vector<double> parameters);
    void ChangeParasToLayerInfo(vector<double> parameters);

    void ClearPreMat();                     //清除preMatrix的值



    double GetPenatly();                                        //获取折射率的惩罚函数值
    double ComputeT1(double t, vector<double> parameters);      //根据parameters和时间t所对应的透过率
    CoatMatrix ComputeLayerMat(unsigned int layerItem);                  //计算对应膜层的Matrix
    double GetLayerN(unsigned int layerItem);                            //获取对应膜层的折射率
    double GetLayerRefWLN(unsigned int layerItem);                       //获取对应膜层在RefWL时的折射率
    double GetLayerQW(unsigned int layerItem);                           //获取对应膜层的光学厚度
    double GetLayerRate(unsigned int layerItem);                         //获取对应膜层的速率
    string GetLayerControlType(unsigned int layerItem);                  //获取对应膜层的控制方式
    CoatMatrix ComputeNoOptiMat();                              //计算不用于优化的膜层组的Matrix
    CoatMatrix ComputePreCurLayerMat();                         //计算正在镀膜层之前的膜层组的Matrix
    CoatMatrix GetCurLayerMatrix(double t);                     //计算正在镀膜的膜层在时间t的Matrix

    double ComputeCurLight(double t);                           //计算时间点t的透过率
    double ComputeMatrixLight(CoatMatrix totalMatrix);          //计算CoatMatrix的透过率
    double GetCurThkLightVal(double thkQW);                     //计算thkQW对应的透过率
    double GetCurThkPhaseVal(double thkQW);                     //计算thkQW对应的相位
    double GetNextPeakQW();                                     //计算CutPeak对应QW
    void GetInfoVal(double& CutPeakQW,int& nCutPeak,double& Tmax,double&Tmin, double& Tstart,double& Tend,double& Ph_end,double& Tprepeak);
    //获取当前层OMS相关的信息（CutPeak对应QW、拐点数、最大透过率、最小透过率、起始透过率、结束透过率、结束相位、结束前一个Peak对应透过率）

    void GetMaxMinValue(double d1,double d2,double d3,double d4,double& dMax,double& dMin);
    void GetMaxMinValue(double d1,double d2,double d3,double& dMax,double& dMin);

    bool isOutTol();

    //获取折射率
    double GetSubstrateN(string substrateFileName);
    double GetLayerN(string layerFileName,double WL);

};

#endif // FILMDESIGN1_H
