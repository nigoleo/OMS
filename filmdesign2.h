#ifndef FILMDESIGN2_H
#define FILMDESIGN2_H



/**************FilmDesign**********************
//算法：考虑substrateN，N,Rate,_CurLayer,preMatrix
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

class FilmDesign2
{
private:
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

public:
    FilmDesign2();
    void TransRefQWtoMonitorQW(const unsigned int &layerItem);
    void CopyFunc(FilmDesign2* filmDesign2);

    bool GetFileInfo(const string &filePath);
    bool GetFileInfo1(const string &filePath);    //rev1的rcp导入
    bool GetFileInfo2(const string &filePath);    //rev2的rcp导入

    vector<string> Split(string str,string pattern);               //分割string到对应的vector<string>里面
    vector<double> InitiateParameter();                            //初始化parameters
    vector<double> GetParamter(const vector<double> &parameters);
    void ChangeParasToLayerInfo(const vector<double> &parameters);

    void ClearPreMat();                     //清除preMatrix的值

    double GetPenatly();                                        //获取折射率的惩罚函数值
    double ComputeT1(const double &t, const vector<double> &parameters);      //根据parameters和时间t所对应的透过率
    vector<double> GetCurThkDeltaParameters(const double &time,const vector<double> &parameters);       //计算时间time对应的7个参数的偏导数
    CoatMatrix ComputeLayerMat(const unsigned int &layerItem);                  //计算对应膜层的Matrix
    string GetLayerControlType(const unsigned int &layerItem);                  //获取对应膜层的控制方式
    string GetLayerMaterial(const unsigned int &layerItem);                     //获取队应膜层材料名称
    double GetLayerMonitorWL(const unsigned int &layerItem);                    //获取对应膜层的监控波长
    double GetLayerN(const unsigned int &layerItem);                            //获取对应膜层的折射率
    double GetLayerRefWLN(const unsigned int &layerItem);                       //获取对应膜层在RefWL时的折射率
    double GetLayerQW_RefWL(const unsigned int &layerItem);                     //获取对应膜层在RefWL时的膜层光学厚度
    double GetLayerQW(const unsigned int &layerItem);                           //获取对应膜层的光学厚度
    double GetLayerRate(const unsigned int &layerItem);                         //获取对应膜层的速率


    CoatMatrix ComputeNoOptiMat();                              //计算不用于优化的膜层组的Matrix
    CoatMatrix ComputePreCurLayerMat();                         //计算正在镀膜层之前的膜层组的Matrix
    CoatMatrix GetCurLayerMatrix(const double &t);                     //计算正在镀膜的膜层在时间t的Matrix

    double ComputeCurLight(const double &t);                           //计算时间点t的透过率
    double ComputeMatrixLight(CoatMatrix totalMatrix);          //计算CoatMatrix的透过率
    double GetCurThkLightVal(const double &thkQW);                     //计算thkQW对应的透过率

    double GetCurThkPhaseVal(const double &thkQW);                     //计算thkQW对应的相位
    double GetNextPeakQW(const double &thkQW);                         //本层镀了thkQW后计算达到CutPeak对应QW
    void GetInfoVal(double& CutPeakQW,int& nCutPeak,double& Tmax,double&Tmin, double& Tstart,double& Tend,double& Ph_end,double& Tprepeak);
    //获取当前层OMS相关的信息（CutPeak对应QW、拐点数、最大透过率、最小透过率、起始透过率、结束透过率、结束相位、结束前一个Peak对应透过率）
    void GetInfoVal1(double& CutPeakQW,int& nCutPeak,double& Tend);
    //获取当前层OMS相关的信息（CutPeak对应QW、拐点数、结束透过率)
    void GetInfoVal2(const double& finishedQW,double& NextPeakQW,double &TnextPeak,double &T2ndPeak);
    //获取当前层OMS相关的信息（该层已完成的finishedQW后,该层完成finishedQW后继续镀膜到达极值点所需的QW）

    void GetMaxMinValue(const double &d1,const double &d2,const double &d3,const double &d4,double& dMax,double& dMin);
    void GetMaxMinValue(const double& d1,const double &d2,const double &d3,double& dMax,double& dMin);

    bool isOutTol();

    //获取折射率
    double GetSubstrateN(const string &substrateFileName);
    double GetLayerN(const string &layerFileName,const double &WL);

    //获取private数据
    unsigned int GetCurLayer();
    string GetDesignName();
    double GetHN();
    string GetHName();
    string GetLName();
    double GetLN();
    double GetHRate();
    double GetLRate();
    bool GetIsNotNull();
    int GetTotalLayer();
    double GetRefWL();
    double GetOMSWL();
    double Get_curQW();
    //设置private数据
    void SetCurLayer(const int &nval);

};


#endif // FILMDESIGN2_H
