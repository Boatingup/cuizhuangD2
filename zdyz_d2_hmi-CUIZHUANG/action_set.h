#ifndef ACTION_SET_H
#define ACTION_SET_H

#include <QDialog>
#include <QTextCodec>

#include "./threads/keyboardreadthread.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "./threads/para_synchronize_thread.h"

#include "widget_stack.h"

namespace Ui {
class action_set;
}

class action_set : public QDialog
{
    Q_OBJECT

public:
    explicit action_set(int index,QWidget *parent = nullptr);
    ~action_set();
    bool IsNumNotinSide(float num);
    bool IsNumBetween(float num);
    ActionRegister* action_register;
    int getActionByindex(int index);
protected:
    bool event(QEvent *event);

private:
    Ui::action_set *ui;

private:
    QString action_key;
    QString action_value;
    QString english_action;

    QString str_category;
    int tab;
    int row;

    Parameters* para;
    ParaSynchronizeThread* pst;

    QString str_temp;
    QString parameter_name;

    int parameter_type;

    int list_range_set_index;
    QString new_set_value;
    int is_global;

    QString str_send;

    int action_index;
    int is_number = 0;
    int ok_step; //indicator at which step of key ok, in step 0, press ok then confirm the para, in the step 1, press ok then sync para.

public:
    void set_category_string(QString& s) {
        str_category = s;
    }

    void set_row(int r){
        row = r;
    }

public:
    CommAcsSocketThread* comm_acs_thread;

private:
    WidgetStack* widget_stack;
    //QString strlow,strhigh;
    float low,high;

};

#endif // ACTION_SET_H
