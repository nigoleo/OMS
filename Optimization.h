#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include <vector>
#include <iostream>
#include <Eigen/Geometry>
#include "filmdesign2.h"
using namespace Eigen;
using namespace std;



using namespace::std;
class Optimization
{
public:
    const double DERIV_STEP = 1e-5;
    const int MAX_ITER = 30;
    const double M_PI_2 = 1.570796327;

    int penaltyIndex = 1;  //惩罚函数的系数

    FilmDesign2* Design1;

    VectorXd input;
    VectorXd output;
    VectorXd params;

    Optimization();
    void GetFilmDesign(FilmDesign2* design);

    void Initiate(const vector<double> input1, const vector<double> output1, const vector<double> params1);

    //获取对应的信息
    //根据输入时间和参数值获取模拟曲线的透过率
    vector<double> CalSimulatedOutput(vector<double>input,vector<double>parameter);

    //计算某个时间点的模拟透过率与实际透过率偏差值
    double func(const VectorXd& input, const VectorXd& output, const VectorXd& parameter,int objIndex);
    VectorXd objF(const VectorXd& input, const VectorXd& output,const VectorXd& parameter);
    double Func(const VectorXd& obj);
    double Deriv(const VectorXd& input, const VectorXd& output, int objIndex, const VectorXd& params,
                 int paraIndex,const VectorXd& obj);

    MatrixXd Jacobin(const VectorXd& input,const VectorXd& output,const VectorXd& params,const VectorXd& obj);

    double maxMatrixDiagonale(const MatrixXd& Hessian);
    double linerDeltaL(const VectorXd& step, const VectorXd& gradient, const double u);
    //使用前需要先Initiate，获取对应的信号数据以及FilmDesign的数据
    void gaussNewton();
    void levenMar();
    void dogLeg();
};

#endif // COATDESIGN_H
