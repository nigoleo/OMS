#include "filmdesign2.h"


FilmDesign2::FilmDesign2()
{
    isNoNull = false;
    isBackSideNull = false;

    _curQW = 0;
    _curQW_Orignal = 0;

}
bool FilmDesign2::GetFileInfo(string filePath)
{
    bool bRet = false;
    try
    {
        fstream fs;
        fs.open(filePath);
        string line;
        getline(fs,line);   //第一行版本信息
        vector<string> tempList = Split(line,"*");
        fs.close();
        if(tempList[1] == "1")
        {
            bRet = GetFileInfo1(filePath);
        }
        else if (tempList[1] == "mwl")
        {
            bRet = GetFileInfo2(filePath);
        }
    }
    catch(exception ex)
    {
    }
    return bRet;
}

bool FilmDesign2::GetFileInfo1(string filePath)
{
    bool bRet = false;
    try
    {
        fstream fs;
        fs.open(filePath);
        string line;
        getline(fs,line);   //第一行版本信息
        getline(fs,line);   //第二行名称
        vector<string> tempList = Split(line,"*");
        this->designName = tempList[1];
        getline(fs,line);   //第三行参考波长/基底材料/基底折射率
        tempList = Split(line,"*");
        OMSWL = atof(tempList[1].c_str());  //string强制转化为double
        refWL = OMSWL;
        subStrateName = tempList[2];
        subStrateN = GetSubstrateN(subStrateName);
        subStrateN_Orignal = subStrateN;

        for (int i = 0; i < 2; i++)//第4/5行材料的相关信息
        {
            getline(fs,line);
            tempList = Split(line,"*");
            if(tempList[2] == "H")
            {
                HName = tempList[3];
                HRate = atof(tempList[4].c_str());
                HN = atof(tempList[5].c_str());
                HN_Original = HN;
                HRate_Orignal = HRate;
            }
            else
            {
                LName = tempList[3];
                LRate = atof(tempList[4].c_str());
                LN = atof(tempList[5].c_str());
                LN_Original = LN;
                LRate_Orignal = LRate;
            }
        }
        indexT = 1;

        getline(fs,line);   //第六行总行数
        tempList = Split(line,"*");
        totalLayer = atoi(tempList[1].c_str());
        getline(fs,line);   //第六行总行数

        for (int i = 0; i < totalLayer; i++)
        {
            getline(fs,line);
            tempList = Split(line,"*");
            layerMat.push_back(tempList[3]);
            layerQW.push_back(atof(tempList[4].c_str()));
            layerControlType.push_back(tempList[14]);
            layerMonitorWL.push_back(OMSWL);    //每层的监控波长
        }
        layerQW_RefWL = layerQW;      //refWL对应的QW
        curLayer = 0;
        isNoNull = true;
        bRet = true;
        fs.close();
    }//try
    catch(exception ex)
    {
      //
    }
    return bRet;
}


bool FilmDesign2::GetFileInfo2(string filePath)
{
    bool bRet = false;
    try
    {
        fstream fs;
        fs.open(filePath);
        string line;
        getline(fs,line);   //第一行版本信息
        getline(fs,line);   //第二行名称
        vector<string> tempList = Split(line,"*");
        this->designName = tempList[1];
        getline(fs,line);   //第三行参考波长/基底材料/基底折射率
        tempList = Split(line,"*");
        OMSWL = atof(tempList[1].c_str());  //string强制转化为double
        refWL = OMSWL;
        subStrateName = tempList[2];
        subStrateN = GetSubstrateN(subStrateName);
        subStrateN_Orignal = subStrateN;

        for (int i = 0; i < 2; i++)//第4/5行材料的相关信息
        {
            getline(fs,line);
            tempList = Split(line,"*");
            if(tempList[2] == "H")
            {
                HName = tempList[3];
                HRate = atof(tempList[4].c_str());
                HN = atof(tempList[5].c_str());
                HN_Original = HN;
                HRate_Orignal = HRate;
            }
            else
            {
                LName = tempList[3];
                LRate = atof(tempList[4].c_str());
                LN = atof(tempList[5].c_str());
                LN_Original = LN;
                LRate_Orignal = LRate;
            }
        }
        indexT = 1;

        getline(fs,line);   //第六行总行数
        tempList = Split(line,"*");
        totalLayer = atoi(tempList[1].c_str());
        getline(fs,line);   //第六行总行数

        for (int i = 0; i < totalLayer; i++)
        {
            getline(fs,line);
            tempList = Split(line,"*");
            layerMat.push_back(tempList[2]);
            layerQW_RefWL.push_back(atof(tempList[3].c_str()));
            layerControlType.push_back(tempList[11]);
            layerMonitorWL.push_back(atof(tempList[6].c_str()));    //每层的监控波长
            layerQW.push_back(atof(tempList[7].c_str()));
        }
        curLayer = 0;
        isNoNull = true;
        bRet = true;
        fs.close();
    }//try
    catch(exception ex)
    {
      //
    }
    return bRet;
}

void FilmDesign2::TransRefQWtoMonitorQW(unsigned int layerItem)
{
    if(OMSWL == layerMonitorWL[layerItem-1])
    {
        return;
    }
    OMSWL = layerMonitorWL[layerItem-1];
    subStrateN = GetSubstrateN(subStrateName);
    subStrateN_Orignal = subStrateN;

    //修改HN/LN的折射率
    //需要添加一个获取折射率的函数

    double n = GetLayerN(HName,OMSWL);
    if (n != 1.0)
    {
        HN = n;
    }
    n = GetLayerN(LName,OMSWL);
    if (n != 1.0)
    {
        LN = n;
    }


    for(unsigned int i = 0; i < this->totalLayer;i++)
    {
        double layerN = GetLayerN(i);
        double monitorN = GetLayerRefWLN(i);
        double QW = layerQW_RefWL[i];
        layerQW[i] = QW*refWL/monitorN*layerN/OMSWL;
    }
    //根据每层的N和refWL \QW_ref 转化为 OMSWL和 QW_Monitor
}
//分割字符串
vector<string> FilmDesign2::Split(string str,string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str+=pattern;//扩展字符串以方便操作
    unsigned int size=str.size();

    for(unsigned int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}


//初始化parameters
vector<double> FilmDesign2::InitiateParameter()
{
    vector<double>().swap(checkParams);
    checkParams.push_back(subStrateN);
    if(layerMat[curLayer - 1] == HName)
    {
        checkParams.push_back(HN);
        checkParams.push_back(HRate);
    }
    else
    {
        checkParams.push_back(LN);
        checkParams.push_back(LRate);
    }

    checkParams.push_back(preTotalMatrix.matrix11);
    checkParams.push_back(preTotalMatrix.matrix12);
    checkParams.push_back(preTotalMatrix.matrix21);
    checkParams.push_back(preTotalMatrix.matrix22);
//    checkParams.push_back(_curQW);
    return checkParams;
}
vector<double> FilmDesign2::GetParamter(vector<double> parameters)
{
    vector<double>().swap(checkParams);
    checkParams = parameters;
    return checkParams;
}
void FilmDesign2::ChangeParasToLayerInfo(vector<double> parameters)
{
     subStrateN = parameters[0];
//        HN = parameter[1];
//        LN = parameter[2];
     if (layerMat[curLayer - 1] == HName)
     {
         HN = parameters[1];
         HRate = parameters[2];
     }
     else
     {
         LN = parameters[1];
         LRate = parameters[2];
     }

     preTotalMatrix.matrix11 = parameters[3];
     preTotalMatrix.matrix12 = parameters[4];
     preTotalMatrix.matrix21 = parameters[5];
     preTotalMatrix.matrix22 = parameters[6];
     //     _curQW = parameters[3];
}

void FilmDesign2::ClearPreMat()
{
    preTotalMatrix.Initiate();
    preNoOptiMatrix.Initiate();
//    _curQW = 0;
    if (curLayer!=0)
    {
        ComputePreCurLayerMat();
    }
}

//获取折射率的惩罚函数(N,Rate,_curQW)
double FilmDesign2::GetPenatly()
{

    double deltaN = 0;
    double deltaRate = 0;
    double deltaCurQW = 0;
    if (layerMat[curLayer - 1] == HName)
     {
         deltaN = HN-HN_Original;
         deltaRate = HRate-HRate_Orignal;
     }
     else
     {
         deltaN = LN-LN_Original;
         deltaRate = LRate-LRate_Orignal;
     }

    if(abs(deltaN)<=0.5)
    {
        deltaN = 0;
    }
    else
    {
        deltaN = abs(deltaN) - 0.5;
    }

    if(abs(deltaRate)<=2)
    {
        deltaRate = 0;
    }
    else
    {
        deltaRate = abs(deltaN) - 2;
    }

    deltaCurQW = abs(_curQW - _curQW_Orignal);

    if (deltaCurQW<0.8)
    {
        deltaCurQW = 0;
    }
    else
    {
        deltaCurQW = deltaCurQW - 0.8;
    }
    return (deltaN+deltaRate+deltaCurQW)*PenaltyLambda;
}

//根据parameters和时间t所对应的透过率,用于模拟优化的,在这基础上加上折射率的惩罚函数
double FilmDesign2::ComputeT1(double t, vector<double> parameters)
{
    ChangeParasToLayerInfo(parameters);
    return ComputeCurLight(t);
}
//计算对应膜层的Matrix
CoatMatrix FilmDesign2::ComputeLayerMat(unsigned int layerItem)
{
    CoatMatrix tempCoatMatrix;
    if (layerItem <= 0)
    {
        tempCoatMatrix.matrix11 = 1;
        tempCoatMatrix.matrix12 = 0;
        tempCoatMatrix.matrix21 = 0;
        tempCoatMatrix.matrix22 = 1;
    }
    else
    {
        double n = GetLayerN(layerItem);
        double angle = PI / 2 * GetLayerQW(layerItem);
        tempCoatMatrix.matrix11 = cos(angle);
        tempCoatMatrix.matrix12 = sin(angle) / n;
        tempCoatMatrix.matrix21 = sin(angle) * n;
        tempCoatMatrix.matrix22 = cos(angle);
    }
    return tempCoatMatrix;
}
//获取对应膜层的折射率
double FilmDesign2::GetLayerN(unsigned int layerItem)
{
    if (layerMat[layerItem - 1] == HName)
    {
        return HN;
    }
    else
    {
        return LN;
    }
}
//获取对应膜层在RefWL时的折射率
double FilmDesign2::GetLayerRefWLN(unsigned int layerItem)
{
    if (layerMat[layerItem - 1] == HName)
    {
        return HN_Original;
    }
    else
    {
        return LN_Original;
    }
}


//获取对应膜层的光学厚度
double FilmDesign2::GetLayerQW(unsigned int layerItem)
{
    return layerQW[layerItem - 1];
}
//获取对应膜层的速率
double FilmDesign2::GetLayerRate(unsigned int layerItem)
{
    if (layerMat[layerItem - 1] == HName)
     {
         return HRate;
     }
     else
     {
         return LRate;
     }
}

//获取对应膜层的控制方式
string FilmDesign2::GetLayerControlType(unsigned int layerItem)
{
    return layerControlType[layerItem-1];
}
//计算不用于优化的膜层组的Matrix
CoatMatrix FilmDesign2::ComputeNoOptiMat()
{
    CoatMatrix totalMatrix;
    CoatMatrix matrix1;
    CoatMatrix matrix2;
    if (curLayer <= 4)
    {
        totalMatrix.Initiate();
    }
    else
    {
        totalMatrix = ComputeLayerMat(curLayer - 4);
        for (unsigned int i = (curLayer - 4);i > 0; i--)
        {
            matrix1 = totalMatrix;
            matrix2 = ComputeLayerMat(i - 1);
            totalMatrix = matrix1 * matrix2;
        }
    }
    preNoOptiMatrix = totalMatrix;
    return preNoOptiMatrix;
}
//计算正在镀膜层之前的膜层组的Matrix
CoatMatrix FilmDesign2::ComputePreCurLayerMat()
{
    CoatMatrix totalMatrix;
    CoatMatrix matrix1;
    CoatMatrix matrix2;

    totalMatrix = ComputeLayerMat(curLayer - 1);
    for (unsigned int i = (curLayer - 1);i > 0; i--)
    {
        matrix1 = totalMatrix;
        matrix2 = ComputeLayerMat(i - 1);
        totalMatrix = matrix1 * matrix2;
    }

    preTotalMatrix = totalMatrix;
    return preTotalMatrix;
}
//计算正在镀膜的膜层在时间t的Matrix
CoatMatrix FilmDesign2::GetCurLayerMatrix(double t)
{
    //获取单前层的折射率
    CoatMatrix tempCoatMatrix;
    double n = GetLayerN(curLayer);
    double rate = GetLayerRate(curLayer);
    //获取单前层的速率
    double angle = PI / 2 * (_curQW + rate*t*n / 2.5/ OMSWL);
    tempCoatMatrix.matrix11 = cos(angle);
    tempCoatMatrix.matrix12 = sin(angle) / n;
    tempCoatMatrix.matrix21 = sin(angle) * n;
    tempCoatMatrix.matrix22 = cos(angle);
    return tempCoatMatrix;
}
//计算时间点t的透过率
double FilmDesign2::ComputeCurLight(double t)
{
    CoatMatrix totalMatrix = GetCurLayerMatrix(t)* preTotalMatrix;
    return ComputeMatrixLight(totalMatrix);
}
//计算CoatMatrix的透过率
double FilmDesign2::ComputeMatrixLight(CoatMatrix totalMatrix)
{
    if (!isBackSideNull)   //需要考虑背面
    {
        double T1 = 4 * subStrateN / (pow((totalMatrix.matrix11 + totalMatrix.matrix22 * subStrateN), 2) + pow((totalMatrix.matrix12 * subStrateN + totalMatrix.matrix21), 2));
        double T2 = (1 - pow(((subStrateN - 1) / (subStrateN + 1)), 2));
        return indexT*100 * (T2 * T1/(1-(1-T1)*(1-T2)));
    }
    else
    {
        return indexT * 100 * 4 * subStrateN / (pow((totalMatrix.matrix11 + totalMatrix.matrix22 * subStrateN), 2) + pow((totalMatrix.matrix12 * subStrateN + totalMatrix.matrix21), 2));
    }
}
//计算thkQW对应的透过率
double FilmDesign2::GetCurThkLightVal(double thkQW)
{
    CoatMatrix tempCoatMatrix;
    double n = GetLayerN(curLayer);
    //获取单前层的速率
    double angle = PI /2 * (thkQW); //thkQW包含了_curQW
    tempCoatMatrix.matrix11 = cos(angle);
    tempCoatMatrix.matrix12 = sin(angle) / n;
    tempCoatMatrix.matrix21 = sin(angle) * n;
    tempCoatMatrix.matrix22 = cos(angle);
    /*if (preTotalMatrix.isNull())
    {
        ComputePreCurLayerMat();
    }*/
    CoatMatrix totalMatrix = tempCoatMatrix* preTotalMatrix;
    return ComputeMatrixLight(totalMatrix);
}
//计算thkQW对应的相位
double FilmDesign2::GetCurThkPhaseVal(double thkQW)
{
    CoatMatrix tempCoatMatrix ;
    double n = GetLayerN(curLayer);
    //获取单前层的速率
    double angle = PI / 2 * (thkQW);   //thkQW包含了_curQW
    tempCoatMatrix.matrix11 = cos(angle);
    tempCoatMatrix.matrix12 = sin(angle) / n;
    tempCoatMatrix.matrix21 = sin(angle) * n;
    tempCoatMatrix.matrix22 = cos(angle);
    //if(preTotalMatrix == null)
    //{
    //    ComputePreCurLayerMat();
    //}
    CoatMatrix totalMatrix = tempCoatMatrix* preTotalMatrix;
    CoatMatrix subStrate;
    subStrate.matrix11 = 1;
    subStrate.matrix12 = 0;
    subStrate.matrix21 = 0;
    subStrate.matrix22 = subStrateN;
    totalMatrix = totalMatrix* subStrate;

    double DR, DI, EI, ER, DDx_minus_EEx, dblDesignWLPhase;

    DR = totalMatrix.matrix11;
    DI = totalMatrix.matrix12;
    EI = totalMatrix.matrix21;
    ER = totalMatrix.matrix22;

    DDx_minus_EEx = DR * DR + DI * DI - ER * ER - EI * EI;
//    dblDesignWLPhase = (2 * DI * ER - 2 * DR * EI) / DDx_minus_EEx;
    dblDesignWLPhase = atan2((2 * DI * ER - 2 * DR * EI) ,DDx_minus_EEx);
    dblDesignWLPhase = dblDesignWLPhase / PI * 180;

/*    if(((DDx_minus_EEx) < 0) &&((2 * DI * ER - 2 * DR * EI) < 0))
      dblDesignWLPhase = dblDesignWLPhase - 180;
    else if (((DDx_minus_EEx) < 0)&&((2 * DI * ER - 2 * DR * EI) >= 0))
      dblDesignWLPhase = dblDesignWLPhase + 180;*/

    if (dblDesignWLPhase < 0)
        dblDesignWLPhase = dblDesignWLPhase + 360;

    return dblDesignWLPhase;
}
//计算CutPeak对应QW
double FilmDesign2::GetNextPeakQW()
{
    double DR,DI,ER,EI;
    double aa,bb,cc,dd;
    double a,b,c;
    double nk = GetLayerN(curLayer);
    DR = preTotalMatrix.matrix11;
    DI = preTotalMatrix.matrix12;
    EI = preTotalMatrix.matrix21;
    ER = preTotalMatrix.matrix22;

    aa = DR + ER * subStrateN ;
    bb = (-1) * (EI / nk + DI * nk * subStrateN);
    cc = subStrateN * DI + EI;
    dd = DR * nk + ER * subStrateN / nk;

    a = aa * bb + cc * dd;
    b = dd * dd - cc * cc - aa * aa + bb * bb;
    c = -a;

    if (a == 0)
    {
        return 1;
    }
    double x1, x2;
    x1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
    x2 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

    double alpha;
    double Nqw;
    if (x1 < 0)
    {
        if (abs(x1)<0.001)
        {
            Nqw = 1;
            return Nqw;
        }
        else
        {
            alpha = atan(x1);
        }
    }
    else
    {
        if (abs(x2)<0.001)
        {
            Nqw = 1;
            return Nqw;
        }
        else
        {
            alpha = atan(x2);
        }
    }

    Nqw = -2 * alpha / PI;
    if (abs(Nqw - 1) < 0.001)
    {
        Nqw = 1;
    }
    return Nqw;
}
//获取当前层OMS相关的信息（CutPeak对应QW、拐点数、最大透过率、最小透过率、起始透过率、结束透过率、结束相位、结束前一个Peak对应透过率）
void FilmDesign2::GetInfoVal(double& CutPeakQW,int& nCutPeak,double& Tmax,double&Tmin, double& Tstart,double& Tend,double& Ph_end,double& Tprepeak)
{
    CutPeakQW = GetNextPeakQW();
    double QW_theory = layerQW[curLayer-1];
    //nCutPeak
    nCutPeak = 0;
    while (CutPeakQW <= QW_theory)
    {
        nCutPeak++;
        CutPeakQW++;
        //QW_theory = QW_theory - nCutPeak;
    }

    double T_firstPeak = GetCurThkLightVal(CutPeakQW);
    double T_doublePeak = GetCurThkLightVal(CutPeakQW + 1);
    Tstart = GetCurThkLightVal(0);
    Tend = GetCurThkLightVal(layerQW[curLayer - 1]);
    Ph_end = GetCurThkPhaseVal(layerQW[curLayer - 1]);
    //Tmax,Tmin,Tprepeak

    Tprepeak = T_doublePeak;

    if (nCutPeak ==0)
    {
        if (Tstart>Tend)
        {
            Tmax = Tstart;
            Tmin = Tend;
        }
        else
        {
            Tmax = Tend;
            Tmin = Tstart;
        }
        Tprepeak = Tstart;
    }
    else if (nCutPeak == 1)
    {
        GetMaxMinValue(Tstart,Tend,T_doublePeak,Tmax,Tmin);
    }
    else
    {
        GetMaxMinValue(Tstart,Tend,T_firstPeak,T_doublePeak,Tmax,Tmin);
    }
}

void FilmDesign2::GetMaxMinValue(double d1,double d2,double d3,double d4,double& dMax,double& dMin)
{
    dMax = d1;
    dMin = d1;
    if(dMax < d2)
    {
        dMax = d2;
    }
    if (dMax < d3)
    {
        dMax = d3;
    }
    if (dMax < d4)
    {
        dMax = d4;
    }

    if (dMin > d2)
    {
        dMin = d2;
    }
    if (dMin > d3)
    {
        dMin = d3;
    }
    if (dMin > d4)
    {
        dMin = d4;
    }
}

void FilmDesign2::GetMaxMinValue(double d1,double d2,double d3,double& dMax,double& dMin)
{
    dMax = d1;
    dMin = d1;
    if(dMax < d2)
    {
        dMax = d2;
    }
    if (dMax < d3)
    {
        dMax = d3;
    }

    if (dMin > d2)
    {
        dMin = d2;
    }
    if (dMin > d3)
    {
        dMin = d3;
    }

}

bool FilmDesign2::isOutTol()
{
    double deltaN = 0;
    double deltaRate = 0;
    double deltaSubN = 0;
    if (layerMat[curLayer - 1] == HName)
     {
         deltaN = HN-HN_Original;
         deltaRate = HRate-HRate_Orignal;
     }
     else
     {
         deltaN = LN-LN_Original;
         deltaRate = LRate-LRate_Orignal;
     }
    deltaSubN = subStrateN - subStrateN_Orignal;
    if(abs(deltaN)>=0.3)
    {
        return true;
    }
    if (abs(deltaRate)>1)
    {
        return true;
    }
    if (abs(deltaSubN)>=0.5)
    {
        return true;
    }
    if (abs(_curQW - _curQW_Orignal)>=0.5)
    {
        return true;
    }
    return false;
}

double FilmDesign2::GetSubstrateN(string substrateFileName)
{
    try
    {
        QString FileAddress = QCoreApplication::applicationDirPath();
        QString logFileName = FileAddress + "//" + QString::fromStdString(substrateFileName) + ".txt";
        QFile f(logFileName);

        if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return 1.52629204;
        }

        QTextStream txtInput(&f);
        QString lineStr;
        QStringList strList;
        vector<double> wl;
        vector<double> n;
        while(!txtInput.atEnd())
        {
            try {
                lineStr = txtInput.readLine();
                strList = lineStr.split("   ");
                wl.push_back(strList[0].toDouble());
                n.push_back(strList[1].toDouble());
            } catch (exception ex) {
                return 1.52629204;
            }
        }
        if (OMSWL<=wl[0])
        {
            return n[0];
        }
        else if (OMSWL>=wl[wl.size()-1])
        {
            return n[n.size()-1];
        }
        else
        {
            for (unsigned int i =0; i<wl.size()-1;i++)
            {
                if ((OMSWL>wl[i])&&(OMSWL<=wl[i+1]))
                {
                    return n[i]+(OMSWL-wl[i])*(n[i+1]-n[i])/(wl[i+1]-wl[i]);
                }
            }
        }
    }
    catch(exception ex)
    {
        return 1.52629204;
    }
    return 1.52629204;
}
double FilmDesign2::GetLayerN(string layerFileName,double WL)
{
    try
    {
        QString FileAddress = QCoreApplication::applicationDirPath();
        QString logFileName = FileAddress + "//" + QString::fromStdString(layerFileName) + ".txt";
        QFile f(logFileName);

        if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return 1.0;
        }

        QTextStream txtInput(&f);
        QString lineStr;
        QStringList strList;
        vector<double> wl;
        vector<double> n;
        while(!txtInput.atEnd())
        {
            try {
                lineStr = txtInput.readLine();
                strList = lineStr.split("   ");
                wl.push_back(strList[0].toDouble());
                n.push_back(strList[1].toDouble());
            } catch (exception ex) {
                return 1.0;
            }
        }
        if (WL<=wl[0])
        {
            return n[0];
        }
        else if (WL>=wl[wl.size()-1])
        {
            return n[n.size()-1];
        }
        else
        {
            for (unsigned int i =0; i<wl.size()-1;i++)
            {
                if ((WL>wl[i])&&(WL<=wl[i+1]))
                {
                    return n[i]+(WL-wl[i])*(n[i+1]-n[i])/(wl[i+1]-wl[i]);
                }
            }
        }
    }
    catch(exception ex)
    {
        return 1.0;
    }
    return 1.0;
}
