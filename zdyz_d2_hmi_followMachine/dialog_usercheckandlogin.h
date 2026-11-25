#ifndef DIALOG_USERCHECKANDLOGIN_H
#define DIALOG_USERCHECKANDLOGIN_H

#include <QDialog>
#include <QFile>
#include "./Resources/key_value.h"
#include "./Resources/gui_setting.h"
#include "./threads/para_synchronize_thread.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
namespace Ui {
class Dialog_UserCheckAndLogin;
}

class Dialog_UserCheckAndLogin : public QDialog
{
    Q_OBJECT

public:
    //explicit Dialog_UserCheckAndLogin(int flag,QString switch_name,QWidget *parent = nullptr);
    explicit Dialog_UserCheckAndLogin(int flag,QString switch_name,bool * switch_flag,QWidget *parent = nullptr);
    ~Dialog_UserCheckAndLogin();

private:
    Ui::Dialog_UserCheckAndLogin *ui;
    int verify_flag;
    bool combox_select = false;
    int last_combox_index;
    CommAcsSocketThread * comm_acs_thread;
    QString str_temp;
    WidgetStack* widget_stack;
    int index = 0;
    QString name_switch;
    Parameters* para;
    gui_setting *gui_set;
    bool * p_switch_flag;
    int index_value;//选定的值
    ParaSynchronizeThread *pst;
protected:
    bool event(QEvent *event);
signals:
    void change_user();
    void change_result(int result);
};

#endif // DIALOG_USERCHECKANDLOGIN_H
