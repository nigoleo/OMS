#ifndef COATMATRIX_H
#define COATMATRIX_H


class CoatMatrix
{
public:
    double matrix11;
    double matrix12;
    double matrix21;
    double matrix22;

    CoatMatrix();
    CoatMatrix(double m11,double m12,double m21,double m22);
    void Initiate();
    bool isNull();          //用于判定是否为空，如果为初始值就默认为未经过计算
    CoatMatrix Multi(CoatMatrix m_CM1,CoatMatrix m_CM2);
    CoatMatrix operator *(CoatMatrix m_CoatMatrix);

};

#endif // COATMATRIX_H
