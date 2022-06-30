#ifndef DATASOLUTION_H
#define DATASOLUTION_H

#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
#include <iostream>
using namespace std;

class DataSolution
{
private:
    static DataSolution* ptr;
    static string LogAddress;
    static string strLot;
    static string m_Material;
public:
    static DataSolution* getIns(){return ptr;};
private:
    DataSolution();
public:
    bool HandleLog(string &strInfo,int &nLayer,string &strMat,double &dTime,int &nEmission,int &nXS,int &nYS,int &nIGB1,int &nIGB2,int &nIGB3,
                   double &dTrans,double &dRef,double &dDark,double &dCrystal,double &dRate);           //处理Log数据
    int HandleOTFC(string &strInfo,int &nLayer,string &strMat,double &dTime,int &nEmission,int &nXS,int &nYS,int &nIGB1,int &nIGB2,int &nIGB3,
                    double &dTrans,double &dRef,double &dDark,double &dCrystal,double &dRate);          //处理OTFC的数据

    string FormLog(const int &nLayer,const string &strMat,const double &dTime,const int &nEmission,const int &nXS,const int &nYS,const int &nIGB1,const int &nIGB2,const int &nIGB3,
                   const double &dTrans,const double &dRef,const double &dDark,const double &dCrystal,const double &dRate); //生成Log格式的string

    vector<string> Split(string str,string pattern);
    bool IsExistLogAddress();   //LogAddress是否存在
    void SetLogAddress(string address); //设置LogAddress
    void SetStrLot(string strLot);      //设置炉号
    void SetMaterial(string material);  //设置材料的信息

    //保存数据
    void SaveOMSInfo(const unsigned int &coatLayer,const string &strInfo);
    void SaveSimuData(const unsigned int &coatLayer,const vector<double>&simuTime,const vector<double> &simuT);
    void SaveLayerInfo(const unsigned int &coatLayer,const double &crystal,const double &time);
    void SaveLogInfo(const string strInfo);
    void SaveSimuEndTime(const unsigned int &coatLayer,const double &time,const double &Time_cutPeak,const double &Time_Offset,const double &Time_Phase,
                         const double &X2,const double&FittingN,const double&FittingRate);


    bool GetTotalInfo(vector<int> &layerNo,vector<double> &vecCrystal,vector<double> &vecTime); //获取TotalInfo的数据
    bool GetOMSInfo(const int &layer,vector<double> &vecLoadTime,vector<double> &vecLoadCrystal,vector<double> &omsTotalTime,vector<double> &omsTotalT);  //
};

#endif // DATASOLUTION_H
