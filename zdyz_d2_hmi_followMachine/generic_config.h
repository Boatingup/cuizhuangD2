#ifndef GENERIC_CONFIG_H
#define GENERIC_CONFIG_H

#include <QDialog>
#include "widget_stack.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "fault_diagnosis_diaplsy.h"
#include "./Resources/key_value.h"

typedef enum
{
    TYPE_GENERIC_CONNECT_NONE = 0,
    TYPE_GENERIC_CONNECT_BOARD ,
    TYPE_GENERIC_SENSOR_ANGLE ,
    TYPE_GENERIC_SENSOR_HIGH ,
    TYPE_GENERIC_DRIVER_VOLT ,
    TYPE_GENERIC_DRIVER_CUR

}GENERIC_CONFIG_E;

namespace Ui {
class generic_config;
}

class generic_config : public QDialog
{
    Q_OBJECT

public:
    explicit generic_config(GENERIC_CONFIG_E type, QWidget *parent = nullptr);
    ~generic_config();

    void update_driver_chn_config_xml(QString volt, QString cur);
protected:
     bool event(QEvent *event);
private:
    int row;
    int column;
    int type;
private:
    CommAcsSocketThread* comm_acs_thread;

private:
    WidgetStack* widget_stack;

private:
    Ui::generic_config *ui;
};

#endif // GENERIC_CONFIG_H
