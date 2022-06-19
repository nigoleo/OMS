#ifndef COATMATRIX_H
#define COATMATRIX_H


class CoatMatrix
{
private:
    double matrix11;
    double matrix12;
    double matrix21;
    double matrix22;
public:
    CoatMatrix();
    CoatMatrix(const double &m11,const double &m12,const double &m21,const double &m22);
    void Initiate(const double &m11,const double &m12,const double &m21,const double &m22);
    void Initiate();
    bool isNull();          //用于判定是否为空，如果为初始值就默认为未经过计算
    CoatMatrix Multi(const CoatMatrix &m_CM1,const CoatMatrix &m_CM2);
    CoatMatrix operator *(const CoatMatrix &m_CoatMatrix);

    double GetM11();
    double GetM12();
    double GetM21();
    double GetM22();

};

#endif // COATMATRIX_H
