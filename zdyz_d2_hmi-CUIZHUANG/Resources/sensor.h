#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include "mytypes.h"

class Sensor
{
private:
    Sensor();
public:
    static Sensor* getInstance(){
        if(sensor == nullptr){
            sensor = new Sensor();
        }
        return sensor;
    }
public:

    //int             soft_interface_board_version;

    QString         boot_version;
    QString         key_board_version;
    float           pressure[8];
    float           temperature;
    float           humidity;
    float           dust_concentration; // 粉尘浓度
    int             stroke[8];
    int             shearer_position;
    int             height;
    double           pitch_angle;
    double           roll_angle;
    bool            infra_red;
    double           dip_angle[8][3];
    uint16_t    switch_state[6][3];
    //uint16_t        AD[16];
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    bool calc_hight_flag{false};
    float extern_pressure[8];
    int extern_trip[4];
    int master_no;

    int coal_mechine_direction;
    float coal_mechine_speed;
    int coal_mechine_position;

    int phy_info[3][3];
private:
    static Sensor* sensor;
};

#endif // SENSOR_H
