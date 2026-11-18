#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <QString>

class SystemState
{
private:
    SystemState();

public:
    enum System_state_type{
        Initial = 0,
        Normal,
        Controlled,
        RemoteControlled,
        Neighbour,
        Interval,
        Group,
        Stop,
        Lock,
        Update,
        Address,
        Follow,
        Self_pull,
        Human_lock,
        Coal_Put,
        SelfControlled,
        LocalReal
    };
    static QString str_system_state[];


public:
    static SystemState* getInstance();
    System_state_type get_system_state();
    void set_system_state(System_state_type type);

private:
    static SystemState* system_state;
    SystemState::System_state_type ss_type;
};

#endif // SYSTEM_STATE_H
