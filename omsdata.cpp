#include "omsdata.h"

OMSData::OMSData()
{

}


OMSData* OMSData::getInstance(){
    if(m_instance == nullptr){
        m_instance = new OMSData();
    }
    return m_instance;
}
