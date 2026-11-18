#ifndef LOGIN_IN_H
#define LOGIN_IN_H

#include <QWidget>
#include "./Resources/key_value.h"
#include "./Resources/gui_setting.h"
#include "./threads/para_synchronize_thread.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
namespace Ui {
class login_in;
}

class login_in : public QWidget
{
    Q_OBJECT

public:
    explicit login_in(QString name, QWidget *parent = 0);
    ~login_in();

protected:
    bool event(QEvent *event);

signals:
    void change_user();
    void change_result(int result);

private:
    Ui::login_in *ui;

    CommAcsSocketThread * comm_acs_thread;
    QString str_temp;
    WidgetStack* widget_stack;
    QString name_switch;
    Parameters* para;
    gui_setting *gui_set;
    ParaSynchronizeThread *pst;

    int index;
    bool combox_select{false};
    int last_combox_index{0};
};

#endif // LOGIN_IN_H
