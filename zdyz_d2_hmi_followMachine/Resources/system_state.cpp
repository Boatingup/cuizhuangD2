#include "system_state.h"

#include <QString>

SystemState* SystemState::system_state = nullptr;

QString SystemState::str_system_state[] = {
    QString("初始化"),
    QString("空闲"),    ///Normarl
    QString("受控"),
    QString("遥控"),
    QString("邻架"),
    QString("隔架"),
    QString("成组"),
    QString("急停"),
    QString("闭锁"),
    QString("升级"),
    QString("编址"),
    QString("跟机")
};


SystemState::SystemState() : ss_type(SystemState::Normal)
{
}


SystemState* SystemState::getInstance(){
    if(system_state == nullptr){
        system_state = new SystemState();
    }
    return system_state;
}

SystemState::System_state_type SystemState::get_system_state(){
    return this->ss_type;
}

void SystemState::set_system_state(SystemState::System_state_type type){
    this->ss_type = type;
}
