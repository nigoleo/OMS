#ifndef OMSDATA_H
#define OMSDATA_H


class OMSData
{

public:
    static OMSData* getInstance();
    static void cleanInstance();
    int GetValue();
    void SetValue(int iValue);
private:
    static OMSData* m_instance;
    int m_iValue;
    OMSData();
    ~OMSData();

};

OMSData* OMSData::m_instance = nullptr;
#endif // OMSDATA_H
