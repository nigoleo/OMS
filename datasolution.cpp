#include "datasolution.h"
DataSolution* DataSolution::ptr = new DataSolution;
string DataSolution::LogAddress = "";
string DataSolution::m_Material = "";
string DataSolution::strLot = "";

DataSolution::DataSolution()
{
}

bool DataSolution::HandleLog(string &strInfo,int &nLayer,string &strMat,double &dTime,int &nEmission,int &nXS,int &nYS,int &nIGB1,int &nIGB2,int &nIGB3,
               double &dTrans,double &dRef,double &dDark,double &dCrystal,double &dRate)
{
    vector<string> strList = Split(strInfo,",");
    vector<string>::size_type pos = strList.size();
    if(pos!= 14)
        return false;
    try
    {
        nLayer = stoi(strList[0]);
        strMat = strList[1];
        dTime = stod(strList[2]);
        nEmission = stoi(strList[3]);
        nXS = stoi(strList[4]);
        nYS = stoi(strList[5]);
        nIGB1 = stoi(strList[6]);
        nIGB2 = stoi(strList[7]);
        nIGB3 = stoi(strList[8]);
        dTrans = stod(strList[9]);
        dRef = stod(strList[10]);
        dDark = stod(strList[11]);
        dCrystal = stod(strList[12]);
        dRate = stod(strList[13]);
    }
    catch(exception ex)
    {
        return false;
    }
    return true;

}
int DataSolution::HandleOTFC(string &strInfo,int &nLayer,string &strMat,double &dTime,int &nEmission,int &nXS,int &nYS,int &nIGB1,int &nIGB2,int &nIGB3,
                double &dTrans,double &dRef,double &dDark,double &dCrystal,double &dRate)
{

    if(strInfo == "\n")
    {   return 0;}           //为空数据

    vector<string> strList = Split(strInfo,",");

    if(strList.size()==4)
    {
        for(auto item:strList)
        {
            vector<string> info = Split(item,"=");
            if(info[0] == "L")
                nLayer = stoi(info[1]);
            else if(info[0] == "Mat")
                SetMaterial(info[1].substr(0,1));
        }
        return 1;       //为层开始信号
    }
    else if (strList.size()==14)
    {
        for(auto item:strList)
        {
            vector<string> info = Split(item,"=");
            strMat = m_Material;
            if(info[0] == "L") nLayer = stoi(info[1]);
            else if(info[0] == "T") dTime = stod(info[1]);
            else if(info[0] == "Emi") nEmission = stoi(info[1]);
            else if(info[0] == "XS") nXS = stoi(info[1]);
            else if(info[0] == "YS") nYS = stoi(info[1]);
            else if(info[0] == "IBG1") nIGB1 = stoi(info[1]);
            else if(info[0] == "IBG2") nIGB2 = stoi(info[1]);
            else if(info[0] == "IBG3") nIGB3 = stoi(info[1]);
            else if(info[0] == "Li") dTrans = stod(info[1]);
            else if(info[0] == "Ref") dRef = stod(info[1]);
            else if(info[0] == "Dark") dDark = stod(info[1]);
            else if(info[0] == "Th") dCrystal = stod(info[1]);
            else if(info[0] == "Ra") { dRate = stod(info[1]); if (abs(dRate)>10000) dRate /= 10000;}
        }
        return 2;   //为光控数据
    }
    return 0;
}

string DataSolution::FormLog(const int &nLayer,const string &strMat,const double &dTime,const int &nEmission,const int &nXS,const int &nYS,const int &nIGB1,const int &nIGB2,const int &nIGB3,
               const double &dTrans,const double &dRef,const double &dDark,const double &dCrystal,const double &dRate)
{
    string strInfo = "";
    strInfo = strInfo + to_string(nLayer) + ",";
    strInfo = strInfo + strMat + ",";
    strInfo = strInfo + to_string(dTime) + ",";
    strInfo = strInfo + to_string(nEmission) + ",";
    strInfo = strInfo + to_string(nXS) + ",";
    strInfo = strInfo + to_string(nYS) + ",";
    strInfo = strInfo + to_string(nIGB1) + ",";
    strInfo = strInfo + to_string(nIGB2) + ",";
    strInfo = strInfo + to_string(nIGB3) + ",";
    strInfo = strInfo + to_string(dTrans) + ",";
    strInfo = strInfo + to_string(dRef) + ",";
    strInfo = strInfo + to_string(dDark) + ",";
    strInfo = strInfo + to_string(dCrystal) + ",";
    strInfo = strInfo + to_string(dRate)+"\n";
    return strInfo;
}

vector<string> DataSolution::Split(string str,string pattern)
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

bool DataSolution::IsExistLogAddress()   //LogAddress是否存在
{
    if(LogAddress == "")
        return false;
    else
        return true;
}
void DataSolution::SetLogAddress(string address) //设置LogAddress
{
    this->LogAddress = address;
}
void DataSolution::SetStrLot(string strLot)
{
    this->strLot = strLot;
}
void DataSolution::SetMaterial(string material)
{
    this->m_Material = material;
}

void DataSolution::SaveOMSInfo(const unsigned int &coatLayer,const string &strInfo)
{
    string logFileName = LogAddress + "//Layer_" + to_string(coatLayer) + ".csv";
    ofstream outputText(logFileName,ofstream::out|ofstream::app);
    outputText<<strInfo;
    outputText.close();
}
void DataSolution::SaveSimuEndTime(const unsigned int &coatLayer,const double &time,const double &Time_cutPeak,const double &Time_Offset,const double &Time_Phase,
                                   const double &X2,const double&FittingN,const double&FittingRate)
{
    string logFileName = LogAddress + "//Layer_" + to_string(coatLayer) + "_EndTime.csv";
    ofstream outputText(logFileName,ofstream::out|ofstream::app);
    outputText<<coatLayer<<","<<time<<","<<Time_cutPeak<<","<<Time_Offset<<","<<Time_Phase<<","<<X2<<","<<FittingN<<","<<FittingRate<<"\n";
    outputText.close();
}

void DataSolution::SaveSimuData(const unsigned int &coatLayer,const vector<double> &simuTime,const vector<double> &simuT)
{
    string logFileName = LogAddress + "//Layer_" + to_string(coatLayer) + "_FitData.csv";
    ofstream outputText(logFileName,ofstream::out|ofstream::app);
    try
    {
        for (unsigned int i = 0 ;i<simuTime.size();i++)
        {
            outputText<<coatLayer<<","<<simuTime[i]<<","<<simuT[i]<<"\n";
        }
    }
    catch(exception ex)
    {
        outputText<<coatLayer<<"saveSimulateDataException"<<ex.what();
        std::cout<<coatLayer<<"Fail";
    }
    outputText.close();
}

void DataSolution::SaveLayerInfo(const unsigned int &coatLayer,const double &crystal,const double &time)
{
    string logFileName = LogAddress + "//" + strLot + "_TotalInfo.csv";
    ofstream outputText(logFileName,ofstream::out|ofstream::app);

    outputText<<coatLayer<<","<<crystal<<","<<time<<"\n";

    outputText.close();
}
void DataSolution::SaveLogInfo(const string strInfo)
{
    string logFileName = LogAddress + "//" + strLot + "_Log.csv";
    SYSTEMTIME stime = {0};
    GetLocalTime(&stime);
    string strInfo1 = to_string(stime.wMonth)+ "-" + to_string(stime.wDay)+ " " + to_string(stime.wHour)+ ":" + to_string(stime.wMinute)+ ":" + to_string(stime.wSecond) + "  " +strInfo;

    ofstream outputText(logFileName,ofstream::out|ofstream::app);
    outputText<<strInfo1<<"\n";
    outputText.close();
}
bool DataSolution::GetTotalInfo(vector<int> &layerNo,vector<double> &vecCrystal,vector<double> &vecTime)
{
    bool bRet = true;
    string logFileName = LogAddress + "//" + strLot + "_TotalInfo.csv";
    fstream textInput(logFileName);
    if(!textInput)
    {
        return false;
    }
    string line;
    while(getline(textInput,line))
    {
        vector<string> tempString = Split(line,",");
        if (tempString.size()!=3)
        {
            bRet = false;
            break;
        }
        layerNo.push_back(stoi(tempString[0]));
        vecCrystal.push_back(stod(tempString[1]));
        vecTime.push_back(stod(tempString[2]));
    }
    textInput.close();
    return bRet;
}

bool DataSolution::GetOMSInfo(const int &layer,vector<double> &vecLoadTime,vector<double> &vecLoadCrystal,vector<double> &omsTotalTime,vector<double> &omsTotalT)
{
    bool bRet = true;
    string logFileName = LogAddress + "//Layer_" + to_string(layer) + ".csv";
    fstream textInput(logFileName);
    if(!textInput)
    {
        return false;
    }
    vector<double>().swap(vecLoadTime);
    vector<double>().swap(vecLoadCrystal);
    vector<double>().swap(omsTotalTime) ;
    vector<double>().swap(omsTotalT) ;

    string lineStr;
    vector<string> strList;
    double preTime = 0;
    while(getline(textInput,lineStr))
    {
        try {
            strList = Split(lineStr,",");
            vecLoadTime.push_back(stod(strList[2]));
            vecLoadCrystal.push_back(stod(strList[12]));
            if (vecLoadTime.size()>2)
            {
                if (stod(strList[2]) < vecLoadTime[vecLoadTime.size()-2])
                {
                    preTime += vecLoadTime[vecLoadTime.size()-2];
                }
            }
            omsTotalTime.push_back(stod(strList[2])+preTime);
            omsTotalT.push_back(stod(strList[9]));
        } catch (exception ex) {
            bRet = false;
        }
    }
    textInput.close();
    return bRet;
}
