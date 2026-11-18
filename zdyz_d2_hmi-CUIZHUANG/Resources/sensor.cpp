#include "sensor.h"

#include <QString>
#include <QFile>

Sensor* Sensor::sensor = nullptr;

#define PORT_10_STATUS_NAME "/tmp/sany/port10"
#define PORT_LEFT_STATUS_NAME "/tmp/sany/portleft"
#define PORT_RIGHT_STATUS_NAME "/tmp/sany/portright"

void get_port_info(QString file_name, int *phy_info)
{
    QFile file(file_name);

    if(!file.open(QFile::ReadOnly)){
        return;
    }

    QByteArray data = file.readAll();
    QList<QByteArray> data_list = data.split(',');
    int i;
    for(i = 0; i < data_list.size(); i++){
        QList<QByteArray> child_list = data_list.at(i).split(':');
        if(child_list.size()<2){
            continue;
        }

        if(child_list.at(0).contains("link")){
            if(child_list.at(1).contains("up")){
                phy_info[0] = 1;
            } else {
                phy_info[0] = 0;
            }
        } else if(child_list.at(0).contains("speed")){
            phy_info[1] = child_list.at(1).toInt();
        } else if(child_list.at(0).contains("master")){
            if(child_list.at(1).contains("master")){
                phy_info[2] = 1;
            } else {
                phy_info[2] = 0;
            }
        }
    }
    file.close();

    return ;
}

Sensor::Sensor()
{
    memset(this, 0, sizeof(Sensor));
    for(int i = 0;i < 8;i++)
    {
        this->pressure[i] = -1.0;
    }

    for(int i = 0;i < 8;i++)
    {
        this->stroke[i] = -1;
    }

    this->pitch_angle = -200.0;
    this->roll_angle = -200.0;

    //coal_mechine_direction.clear();
    coal_mechine_direction = 0;
    coal_mechine_speed = 0.0;
    coal_mechine_position = 0;

    memset(phy_info, 0, sizeof(int)*9);

    get_port_info(PORT_10_STATUS_NAME, phy_info[0]);
    get_port_info(PORT_LEFT_STATUS_NAME, phy_info[1]);
    get_port_info(PORT_RIGHT_STATUS_NAME, phy_info[2]);

}
