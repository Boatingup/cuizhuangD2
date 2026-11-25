#ifndef PASSWD_SET_H
#define PASSWD_SET_H

#include <QWidget>
#include <QFile>
#include "./Resources/key_value.h"
#include "./Resources/gui_setting.h"
#include "./threads/para_synchronize_thread.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
namespace Ui {
class passwd_set;
}

class passwd_set : public QWidget
{
    Q_OBJECT

public:
    explicit passwd_set(QString name, QWidget *parent = 0);
    ~passwd_set();

protected:
    bool event(QEvent *event);

private:
    Ui::passwd_set *ui;

    CommAcsSocketThread * comm_acs_thread;
    QString str_temp;
    WidgetStack* widget_stack;
    QString name_switch;
    Parameters* para;
    gui_setting *gui_set;
    ParaSynchronizeThread *pst;

    int index;
};

#endif // PASSWD_SET_H
