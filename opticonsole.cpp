#include "opticonsole.h"

OptiConsole::OptiConsole()
{
    TheoryDesign = new FilmDesign2();
    SimulateDesign = new FilmDesign2();

}

OptiConsole::~OptiConsole()
{
    delete TheoryDesign;
    delete SimulateDesign;
}
void OptiConsole::LoadTheoryDesignToSimualte()
{
    this->SimulateDesign->designName = this->TheoryDesign->designName;
    this->SimulateDesign->isNoNull = this->TheoryDesign->isNoNull;
    this->SimulateDesign->HName = this->TheoryDesign->HName;
    this->SimulateDesign->LName = this->TheoryDesign->LName;
    this->SimulateDesign->HN = this->TheoryDesign->HN;
    this->SimulateDesign->LN = this->TheoryDesign->LN;
    this->SimulateDesign->HRate = this->TheoryDesign->HRate;
    this->SimulateDesign->LRate = this->TheoryDesign->LRate;
    this->SimulateDesign->HN_Original = this->TheoryDesign->HN_Original;
    this->SimulateDesign->LN_Original = this->TheoryDesign->LN_Original;

    this->SimulateDesign->indexT = this->TheoryDesign->indexT;
    this->SimulateDesign->subStrateName = this->TheoryDesign->subStrateName;
    this->SimulateDesign->subStrateN = this->TheoryDesign->subStrateN;
    this->SimulateDesign->totalLayer = this->TheoryDesign->totalLayer;

    this->SimulateDesign->curLayer = this->TheoryDesign->curLayer;
    this->SimulateDesign->OMSWL = this->TheoryDesign->OMSWL;
    this->SimulateDesign->preTotalMatrix = this->TheoryDesign->preTotalMatrix;
    this->SimulateDesign->preNoOptiMatrix = this->TheoryDesign->preNoOptiMatrix;
    this->SimulateDesign->checkParams = this->TheoryDesign->checkParams;

    this->SimulateDesign->_curQW = this->TheoryDesign->_curQW;
    this->SimulateDesign->isBackSideNull = this->TheoryDesign->isBackSideNull;
    this->SimulateDesign->layerMat = this->TheoryDesign->layerMat;
    this->SimulateDesign->layerQW = this->TheoryDesign->layerQW;
    this->SimulateDesign->layerControlType = this->TheoryDesign->layerControlType;

}

void OptiConsole::LoadOMSData(vector<double> vecTime,vector<double> vecT)
{
    vector<double>().swap(vecOMSDataTime);
    vector<double>().swap(vecOMSDataT);

    vecOMSDataTime = vecTime;
    vecOMSDataT = vecT;
}

void OptiConsole::LoadLayerInfo(unsigned int nCurLayer)
{
    TheoryDesign->curLayer = nCurLayer;
    LoadTheoryDesignToSimualte();
}

void OptiConsole::Fitting()
{
    Simulate.GetFilmDesign(SimulateDesign);                                     //获取SimulateDesign
    vector<double> parameters = Simulate.Design1->InitiateParameter();          //初始化参数
    Simulate.Initiate(vecOMSDataTime,vecOMSDataT,parameters);                   //将数据和参数修改为Opti的格式
    Simulate.levenMar();                                                        //LM算法优化，SimulateDesign里面就是新的参数
}


void OptiConsole::GetFittingNR(double &fittingN,double &fittingRate)               //获取拟合的折射率
{
    if(SimulateDesign->layerMat[SimulateDesign->curLayer - 1] == SimulateDesign->HName)
    {
        fittingN = SimulateDesign->HN;
        fittingRate = SimulateDesign->HRate;
        return;
    }
    else
    {
        fittingN = SimulateDesign->LN;
        fittingRate = SimulateDesign->LRate;
        return;
    }
}

vector<double> OptiConsole::GetFittingT(double &X2)
{
    vector<double> simulateT;
    X2 = 0;
    for(unsigned int i = 0; i<vecOMSDataTime.size();i++)
    {
        double T = SimulateDesign->ComputeCurLight(vecOMSDataTime[i]);
        simulateT.push_back(T);
        X2 += pow((T-vecOMSDataT[i]),2);
    }
    X2 = X2/vecOMSDataTime.size()/10000;
    return simulateT;
}

void OptiConsole::GetFittingInfo(double &QW_Phase,double &QW_Offset,double &QW_CutPeak,
                               double &Tend_CutPeak,double &Tend_Offset_theory,double &Tend_Offset_act,double &Tend_Phase_act,
                               double &Pend_theory,double &Phend_act ,double &DeltaPhase,
                               double &Time_CutPeak,double &Time_Offset,double &Time_Phase,
                               double &fittingN,double &fittingRate,double &X2,vector<double> &SimulateT,double &Test)
{
    //计算理论设计当前层的相关数据
    double cutPeakQWNext_theory = 0;     //第一个拐点对应的QW
    int nCutPeak_theory = 0;             //拐点数
    double Tmax_theory = 0;              //最大透过率
    double Tmin_theory = 0;              //最小透过率
    double Tstart_theory = 0;            //起始透过率
    double Tend_theory = 0;              //结束透过率
    double Phend_theory = 0;             //结束相位值
    double Tprepeak_theory = 0;          //结束前的拐点透过率
    TheoryDesign->GetInfoVal(cutPeakQWNext_theory,nCutPeak_theory, Tmax_theory,Tmin_theory,
                     Tstart_theory,Tend_theory,Phend_theory,Tprepeak_theory);

    //计算模拟数据的当前层
    double cutPeakQWNext_act = 0;
    int nCutPeak_act = 0;
    double Tmax_act = 0;
    double Tmin_act = 0;
    double Tstart_act = 0;
    double Tend_act = 0;
    Phend_act = 0;
    double Tprepeak_act = 0;
    SimulateDesign->GetInfoVal(cutPeakQWNext_act, nCutPeak_act, Tmax_act, Tmin_act,
        Tstart_act, Tend_act, Phend_act, Tprepeak_act);

    //CutPeak控制方式
    QW_CutPeak = TheoryDesign->layerQW[TheoryDesign->curLayer-1];
    QW_Offset = TheoryDesign->layerQW[TheoryDesign->curLayer-1];
    QW_Phase = TheoryDesign->layerQW[TheoryDesign->curLayer-1];


    //计算CutPeakNext控制方式所对应的QW和T%end
    if (cutPeakQWNext_act - cutPeakQWNext_theory > 0.5)
    {
        QW_CutPeak = - (1 - cutPeakQWNext_act) + TheoryDesign->layerQW[TheoryDesign->curLayer-1] - cutPeakQWNext_theory;
    }
    else if (cutPeakQWNext_theory - cutPeakQWNext_act>0.5)
    {
        QW_CutPeak = 1 - cutPeakQWNext_theory + cutPeakQWNext_act + TheoryDesign->layerQW[TheoryDesign->curLayer-1];
    }
    else
    {
        QW_CutPeak = TheoryDesign->layerQW[TheoryDesign->curLayer-1] - cutPeakQWNext_theory + cutPeakQWNext_act;
    }

    Tend_CutPeak = SimulateDesign->GetCurThkLightVal(QW_CutPeak);                //CutPeak方式对应的透过率
    //计算目标透过率和目标相位
    Test = Tprepeak_theory;
    Tend_Offset_theory = Tend_theory - Tprepeak_theory + Tprepeak_act;           //Offset方式理论对应的透过率（理论的目标值);如果end离前一个peak太近的话，可能会出现误判
    Pend_theory = Phend_theory;                                                  //Phase方式理论对应的相位值(理论的目标值）

    //计算Offset控制方式所对应的QW和T%end
    //利用GradientDescent方式求Offset的QW
    double stepQW = 0.0001;  //learningRate
//    double eps1 = 1e-14;
    QW_Offset = QW_CutPeak;
    double PreAlpha = GetGradientOffset(QW_Offset,Tend_Offset_theory);
    int nCount = 0;
    double alpha = PreAlpha;
    while (true)
    {
        QW_Offset += stepQW*alpha;
        nCount++;
        alpha = GetGradientOffset(QW_Offset,Tend_Offset_theory);

        if(PreAlpha*alpha <0)
        {
            break;
        }
        if(nCount>10000)
        {
            break;
        }
        if(abs(SimulateDesign->GetCurThkLightVal(QW_Offset)-Tend_Offset_theory)<0.05)
        {
            break;
        }
        if (abs(QW_Offset-QW_CutPeak)>1)
        {
            if (QW_Offset> QW_CutPeak)
            {
                QW_Offset = QW_Offset - 1;
            }
            else
            {
                QW_Offset = QW_Offset + 1;
            }
            break;
        }
    }
    Tend_Offset_act = SimulateDesign->GetCurThkLightVal(QW_Offset);         //Offset方式实际完成的透过率

    /*
    double deltaErr = 999;
    double deltaPreErr = 0;
    double deltaAbsErr = 100;
    double deltaQW = QW_CutPeak;
    double stepQW = 0.001;
    double Trans = 0;
    int changeDirection = 0;
    bool ChangeLayer = false;
    while (!ChangeLayer)
    {
        deltaQW = deltaQW + stepQW;
        Trans = SimulateDesign->GetCurThkLightVal(deltaQW);
        deltaErr = (Trans - Tend_Offset_theory) / Tend_Offset_theory;
        if (deltaQW >= deltaQW+1)
        {
            ChangeLayer = true;
            QW_Offset = deltaQW;
            break;
        }

        if (deltaAbsErr>= abs(deltaErr))
        {
            deltaAbsErr = abs(deltaErr);
        }
        else
        {
            stepQW = stepQW * (-1);
            changeDirection++;
        }
        if (changeDirection == 2)
        {
            QW_Offset = deltaQW;
            break;
        }
        if (deltaPreErr!=0)
        {
            if (deltaErr * deltaPreErr <= 0)
            {
                QW_Offset = deltaQW;
                break;
            }
        }
        deltaPreErr = deltaErr;
    }
    Tend_Offset_act = SimulateDesign->GetCurThkLightVal(QW_Offset);                         //Offset方式实际完成的透过率
*/
    //计算Phase控制方式所对应的QW和T%end以及Phase
    stepQW = 0.0001;  //learningRate
//    eps1 = 1e-12;
    QW_Phase = QW_CutPeak;
    PreAlpha = GetGradientPhase(QW_Phase,Pend_theory);
    alpha = PreAlpha;
    nCount = 0;
    while (true)
    {
        QW_Phase += stepQW*alpha;
        nCount++;
        alpha = GetGradientPhase(QW_Phase,Pend_theory);
        if(PreAlpha*alpha <0)
        {
            break;
        }
        if(nCount>10000)
        {
            break;
        }
        if(abs(SimulateDesign->GetCurThkPhaseVal(QW_Phase)-Pend_theory)<0.1)
        {
            break;
        }
        if (abs(QW_Phase-QW_CutPeak)>1)
        {
            if (QW_Phase> QW_CutPeak)
            {
                QW_Phase = QW_Phase - 1;
            }
            else
            {
                QW_Phase = QW_Phase + 1;
            }
            break;
        }
    }

/*
    deltaErr = 999;
    deltaPreErr = 0;
    deltaAbsErr = 100;
    deltaQW = QW_CutPeak;
    stepQW = 0.001;
    Trans = 0;
    double Ph_oms_act = 0;
    changeDirection = 0;
    ChangeLayer = false;
    while (!ChangeLayer)
    {
        deltaQW = deltaQW + stepQW;
        Ph_oms_act = SimulateDesign->GetCurThkPhaseVal(deltaQW);
        deltaErr = (Ph_oms_act - Phend_theory) / Phend_theory;
        if (deltaQW >= deltaQW + 1)
        {
            ChangeLayer = true;
            QW_Phase = deltaQW;
            break;
        }

        if (deltaAbsErr >= abs(deltaErr))
        {
            deltaAbsErr = deltaErr;
        }
        else
        {
            stepQW = stepQW * (-1);
            changeDirection++;
        }
        if (changeDirection == 2)
        {
            QW_Phase = deltaQW;
            break;
        }
        if (deltaPreErr != 0)
        {
            if (deltaErr * deltaPreErr <= 0)
            {
                QW_Phase = deltaQW;
                break;
            }
        }

        if (abs(deltaErr)<0.5)
        {
            QW_Phase = deltaQW;
            break;
        }
        deltaPreErr = deltaErr;
    }
*/
    Tend_Phase_act = SimulateDesign->GetCurThkLightVal(QW_Phase);                        //phase方式实际完成的透过率
    Phend_act = SimulateDesign->GetCurThkPhaseVal(QW_Phase);                             //phase方式实际完成的相位
    DeltaPhase = abs(Phend_act-Phend_theory);

    QW_CutPeak = QW_CutPeak-SimulateDesign->_curQW;                 //CutPeak方式需要镀膜的厚度
    QW_Offset = QW_Offset-SimulateDesign->_curQW;                   //Offset方式需要镀膜的厚度
    QW_Phase = QW_Phase-SimulateDesign->_curQW;                     //Phase方式需要镀膜的厚度

    GetFittingNR(fittingN,fittingRate);
    SimulateT = GetFittingT(X2);

    Time_CutPeak = QW_CutPeak*SimulateDesign->OMSWL/4/fittingN/fittingRate*10;
    Time_Offset = QW_Offset*SimulateDesign->OMSWL/4/fittingN/fittingRate*10;
    Time_Phase = QW_Phase*SimulateDesign->OMSWL/4/fittingN/fittingRate*10;

}

double OptiConsole::GetGradientOffset(double QW,double T_offset_Theory)
{
    double T1 = abs(SimulateDesign->GetCurThkLightVal(QW-DERIV_STEP) - T_offset_Theory);
    double T2 = abs(SimulateDesign->GetCurThkLightVal(QW+DERIV_STEP) - T_offset_Theory);
    if ((T1-T2)>0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

double OptiConsole::GetGradientPhase(double QW,double Phase_Theory)
{
    double Phase1 = abs(SimulateDesign->GetCurThkPhaseVal(QW-DERIV_STEP) - Phase_Theory);
    double Phase2 = abs(SimulateDesign->GetCurThkPhaseVal(QW+DERIV_STEP) - Phase_Theory);
    if ((Phase1-Phase2)>0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}
