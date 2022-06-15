#include "coatmatrix.h"

CoatMatrix::CoatMatrix()
{
    Initiate();
}

void CoatMatrix::Initiate()
{
    this->matrix11 = 1;
    this->matrix12 = 0;
    this->matrix21 = 0;
    this->matrix22 = 1;
}

bool CoatMatrix::isNull()
{
    if((matrix11 == 1)&&(matrix12 == 0)&&(matrix21 == 0)&&(matrix22 == 1))
    {
        return true;
    }
    else
    {
        return false;
    }
}

CoatMatrix::CoatMatrix(double m11,double m12,double m21,double m22)
{
    this->matrix11 = m11;
    this->matrix12 = m12;
    this->matrix21 = m21;
    this->matrix22 = m22;
}
CoatMatrix CoatMatrix::Multi(CoatMatrix m_CM1,CoatMatrix m_CM2)
{
    double x11 = m_CM1.matrix11*m_CM2.matrix11 - m_CM1.matrix12*m_CM2.matrix21;
    double x12 = m_CM1.matrix11*m_CM2.matrix12 + m_CM1.matrix12*m_CM2.matrix22;
    double x21 = m_CM1.matrix21*m_CM2.matrix11 + m_CM1.matrix22*m_CM2.matrix21;
    double x22 = -m_CM1.matrix21*m_CM2.matrix12 + m_CM1.matrix22*m_CM2.matrix22;

    CoatMatrix ref(x11,x12,x21,x22);
    return ref;

}

CoatMatrix CoatMatrix::operator *(CoatMatrix m_CoatMatrix)
{
    CoatMatrix x;
    x.matrix11 = this->matrix11*m_CoatMatrix.matrix11 - this->matrix12*m_CoatMatrix.matrix21;
    x.matrix12 = this->matrix11*m_CoatMatrix.matrix12 + this->matrix12*m_CoatMatrix.matrix22;
    x.matrix21 = this->matrix21*m_CoatMatrix.matrix11 + this->matrix22*m_CoatMatrix.matrix21;
    x.matrix22 = -this->matrix21*m_CoatMatrix.matrix12 + this->matrix22*m_CoatMatrix.matrix22;
    return x;
}

