#include "velocityaccelerationviewmodel.h"

VelocityAccelerationViewModel::VelocityAccelerationViewModel(QObject *parent) : QObject(parent)
{
    controller = Controller::getInstance();
    if(controller->IsFirstGeneralSettingPage)
    {
        controller->IsFirstGeneralSettingPage = false;
        setConfj(false);
        setConfData(0);
        setSingulPTP(false);
        setSingulCP(false);
    }
    else
    {
        setConfj(controller->Confj);
        setConfData(controller->ConfData);
        setSingulPTP(controller->SingulPTP);
        setSingulCP(controller->SingulCP);
    }
}

bool VelocityAccelerationViewModel::Confj()
{
    _conf_j=controller->Confj;
    return _conf_j;
}

void VelocityAccelerationViewModel::setConfj(bool value)
{
    controller->Confj=value;
    _conf_j=value;
    controller->beckhoff->setConfJ(value);
    Q_EMIT ConfjChanged();
}

int VelocityAccelerationViewModel::ConfData()
{
    _conf_data=controller->ConfData;
    return _conf_data;
}

void VelocityAccelerationViewModel::setConfData(int value)
{
    controller->ConfData=value;
    _conf_data=value;
    controller->beckhoff->setConfData(value);
    Q_EMIT ConfDataChanged();
}

bool VelocityAccelerationViewModel::SingulPTP()
{
    _singul_ptp=controller->SingulPTP;
    return _singul_ptp;
}

void VelocityAccelerationViewModel::setSingulPTP(bool value)
{
    controller->SingulPTP=value;
    _singul_ptp=value;
    controller->beckhoff->setSingulPTP(value);
    Q_EMIT SingulPTPChanged();
}

bool VelocityAccelerationViewModel::SingulCP()
{
    _singul_cp=controller->SingulCP;
    return _singul_cp;
}

void VelocityAccelerationViewModel::setSingulCP(bool value)
{
    controller->SingulCP=value;
    _singul_cp=value;
    controller->beckhoff->setSingulCP(value);
    Q_EMIT SingulCPChanged();
}

