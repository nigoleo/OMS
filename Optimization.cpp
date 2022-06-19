#include "Optimization.h"


Optimization::Optimization()
{

}

void Optimization::GetFilmDesign(FilmDesign2* design)
{
    this->Design1 = design;
}
vector<double> Optimization::FilmDesignInitPara()
{
    return this->Design1->InitiateParameter();
}

void Optimization::Initiate(const vector<double> input1, const vector<double> output1, const vector<double> params1)
{
    input.resize(input1.size());
    output.resize(output1.size());
    params.resize(params1.size());

    for (int i = 0; i < input.size();i++)
	{
		input[i] = input1[i];
	}

	for (int i = 0; i< output.size();i++)
	{
		output[i] = output1[i];
	}

    for (int i = 0; i< params1.size();i++)
	{
		params[i] = params1[i];
	}
}


double Optimization::func(const VectorXd& input, const VectorXd& output, const VectorXd& parameter,int objIndex)
{

    double t = input[objIndex];
    double f = output[objIndex];

    vector<double> params;
    for (int i = 0; i<parameter.size();i++)
    {
        params.push_back(parameter[i]);
    }

//Design1->ComputeT1(t,params);
//如果添加惩罚函数是否添加在func上面
    return Design1->ComputeT1(t,params) - f ;
}


VectorXd Optimization::objF(const VectorXd& input, const VectorXd& output,const VectorXd& parameter)
{
    VectorXd obj(input.size());

    for (int i = 0;i < input.size();i++)
    {
        obj[i] = (func(input, output,parameter,i));
    }

    return obj;
}

double Optimization::Func(const VectorXd& obj)
{
    return obj.norm()/2 ;   //+ this->Design1->GetPenatly()

}

double Optimization::Deriv(const VectorXd& input, const VectorXd& output, int objIndex, const VectorXd& params,
                      int paraIndex,const VectorXd& obj)
{
//可以考虑减少一个obj的计算减少计算量
//    VectorXd para1 = params;
    VectorXd para2 = params;

//    para1(paraIndex) -= DERIV_STEP;
    para2(paraIndex) += DERIV_STEP;

 //   double obj1 = func(input, output, para1, objIndex);
    double obj2 = func(input, output, para2, objIndex);

//    return (obj2 - obj1) / (2 * DERIV_STEP);
    return (obj2 - obj[objIndex]) / DERIV_STEP;

}
MatrixXd Optimization::Jacobin(const VectorXd& input, const VectorXd& output, const VectorXd& params,const VectorXd& obj)
{
    int rowNum = input.size();
    int colNum = params.size();

    MatrixXd Jac(rowNum, colNum);

    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < colNum; j++)
        {
            Jac(i,j) = Deriv(input, output, i, params, j,obj);
        }
    }
    return Jac;
}

void Optimization::gaussNewton()
{
    int errNum = input.size();      //error  num
    int paraNum = params.size();    //parameter  num

    VectorXd obj(errNum);

    double last_sum = 0;

    func(input,output,params,0);

    int iterCnt = 0;
    while (iterCnt < MAX_ITER)
    {
        obj = objF(input, output, params);

        double sum = 0;
        sum = Func(obj);

        if (fabs(sum - last_sum) <= 1e-7)
            break;
        last_sum = sum;

        MatrixXd Jac = Jacobin(input, output, params,obj);
        VectorXd delta(paraNum);
        delta = (Jac.transpose() * Jac).inverse() * Jac.transpose() * obj;

//        if (abs(delta(0))>0.1)
//            delta(0) /= 10;

//        if (abs(delta(1))>0.1)
//            delta(1) /= 10;

        params -= delta;
        iterCnt++;
    }
}


double Optimization::maxMatrixDiagonale(const MatrixXd& Hessian)
{
    double max = 0;
    for(int i = 0; i < Hessian.rows(); i++)
    {
        if(Hessian(i,i) > max)
            max = Hessian(i,i);
    }

    return max;
}

double Optimization::linerDeltaL(const VectorXd& step, const VectorXd& gradient, const double u)
{
    double L = step.transpose() * (u * step - gradient);
    return L/2;
}

void Optimization::levenMar()
{
    int paraNum = params.size();    //parameter num
    //initial parameter
    VectorXd obj = objF(input,output,params);
    MatrixXd Jac = Jacobin(input, output, params,obj);  //jacobin
    MatrixXd A = Jac.transpose() * Jac;             //Hessian
    VectorXd gradient = Jac.transpose() * obj;      //gradient

    //initial parameter tao v epsilon1 epsilon2
    double tao = 1e-3;
    long long v = 2;
    double eps1 = 1e-12, eps2 = 1e-12;
    double u = tao * maxMatrixDiagonale(A);
    bool found = (gradient.norm() <= eps1);
    if(found) return;

    int iterCnt = 0;

    penaltyIndex = 1;

    while (iterCnt < MAX_ITER)
    {
        VectorXd obj = objF(input,output,params);

        MatrixXd Jac = Jacobin(input, output, params,obj);  //jacobin
        MatrixXd A = Jac.transpose() * Jac;             //Hessian
        VectorXd gradient = Jac.transpose() * obj;      //gradient

        if( gradient.norm() <= eps1 )
        {
            break;
        }

        VectorXd step = (A + u * MatrixXd::Identity(paraNum, paraNum)).inverse() * gradient; //negtive Hlm.

        if( step.norm() <= eps2*(params.norm() + eps2) )
        {
            break;
        }

        VectorXd paramsNew(params.rows());
        paramsNew = params - step; //h_lm = -step;

//        if (abs(step(0))>0.1)
//            step(0) /= 10;
//        if (abs(step(1))>0.1)
//            step(1) /= 10;
//        paramsNew = params - step; //h_lm = -step;
//        if ((paramsNew(2)>(DesignRate*1.2))||(params(2)<(DesignRate*0.8)))
//        {
//            paramsNew(2) = paramsNew(2) + step(2);
//        }
        //compute f(x)
        obj = objF(input,output,params);

        //compute f(x_new)
        VectorXd obj_new = objF(input,output,paramsNew);

        double deltaF = Func(obj) - Func(obj_new);
        double deltaL = linerDeltaL(-1 * step, gradient, u);

        double roi = deltaF / deltaL;
        if(roi > 0)
        {
            params = paramsNew;
            u *= max(1.0/3.0, 1-pow(2*roi-1, 3));
            v = 2;
        }
        else
        {
            u = u * v;
            v = v * 2;
        }

        iterCnt++;

        //penaltyIndex *= 2;
    }

}


void Optimization::dogLeg()
{
    VectorXd obj = objF(input, output, params);
    MatrixXd Jac = Jacobin(input, output, params,obj);  //jacobin
    VectorXd gradient = Jac.transpose() * obj;      //gradient

    //initial parameter tao v epsilon1 epsilon2
    double eps1 = 1e-12, eps2 = 1e-12, eps3 = 1e-12;
    double radius = 1.0;

    bool found  = obj.norm() <= eps3 || gradient.norm() <= eps1;
    if(found) return;

    int iterCnt = 0;
    while(iterCnt < MAX_ITER)
    {
        VectorXd obj = objF(input, output, params);
        MatrixXd Jac = Jacobin(input, output, params,obj);  //jacobin
        VectorXd gradient = Jac.transpose() * obj;      //gradient

        if( gradient.norm() <= eps1 )
        {
            break;
        }
        if(obj.norm() <= eps3)
        {
            break;
        }

        //compute how far go along stepest descent direction.
        double alpha = gradient.squaredNorm() / (Jac * gradient).squaredNorm();
        //compute gauss newton step and stepest descent step.
        VectorXd stepest_descent = -alpha * gradient;
        VectorXd gauss_newton = (Jac.transpose() * Jac).inverse() * Jac.transpose() * obj * (-1);

        double beta = 0;

        //compute dog-leg step.
        VectorXd dog_leg(params.rows());
        if(gauss_newton.norm() <= radius)
            dog_leg = gauss_newton;
        else if(alpha * stepest_descent.norm() >= radius)
            dog_leg = (radius / stepest_descent.norm()) * stepest_descent;
        else
        {
            VectorXd a = alpha * stepest_descent;
            VectorXd b = gauss_newton;
            double c = a.transpose() * (b - a);
            beta = (sqrt(c*c + (b-a).squaredNorm()*(radius*radius-a.squaredNorm()))-c)
                    /(b-a).squaredNorm();

            dog_leg = alpha * stepest_descent + beta * (gauss_newton - alpha * stepest_descent);

        }

        if(dog_leg.norm() <= eps2 *(params.norm() + eps2))
        {
            break;
        }

        VectorXd new_params(params.rows());
        new_params = params + dog_leg;

        //compute f(x)
        obj = objF(input,output,params);
        //compute f(x_new)
        VectorXd obj_new = objF(input,output,new_params);

        //compute delta F = F(x) - F(x_new)
        double deltaF = Func(obj) - Func(obj_new);

        //compute delat L =L(0)-L(dog_leg)
        double deltaL = 0;
        if(gauss_newton.norm() <= radius)
            deltaL = Func(obj);
        else if(alpha * stepest_descent.norm() >= radius)
            deltaL = radius*(2*alpha*gradient.norm() - radius)/(2.0*alpha);
        else
        {
            VectorXd a = alpha * stepest_descent;
            VectorXd b = gauss_newton;
            double c = a.transpose() * (b - a);
            beta = (sqrt(c*c + (b-a).squaredNorm()*(radius*radius-a.squaredNorm()))-c)
                    /(b-a).squaredNorm();
            deltaL = alpha*(1-beta)*(1-beta)*gradient.squaredNorm()/2.0 + beta*(2.0-beta)*Func(obj);
        }

        double roi = deltaF / deltaL;
        if(roi > 0)
        {
            params = new_params;
        }
        if(roi > 0.75)
        {
            radius = max(radius, 3.0 * dog_leg.norm());
        }
        else if(roi < 0.25)
        {
            radius = radius / 2.0;
            if(radius <= eps2*(params.norm()+eps2))
            {
                break;
            }
        }
        iterCnt++;
    }
}

