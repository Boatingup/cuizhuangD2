#ifndef PARAMETER_CONFIG_H
#define PARAMETER_CONFIG_H

#include <QDialog>
#include <QTextCodec>
#include <QTableView>

#include "./threads/keyboardreadthread.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "./threads/para_synchronize_thread.h"

#include "widget_stack.h"

namespace Ui {
class ParameterConfig;
}

class ParameterConfig : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterConfig(QWidget *parent = nullptr);

    ParameterConfig(QString& category_name, int tab, int ui_index,int row, QWidget *parent = nullptr);

    ~ParameterConfig();

    bool IsNumNotinSide(float num);
    bool IsNumBetween(float num);
    bool IsFirstRecoilTime();
    void FirstRecoilTimeProc(QString value);
    void updateBaudrateDisplay();
    void updateBaudrateDisplayByIndex();

    void updateIpDisplay();
    void processIpInput(QString value);
    QString getIpValue();

private:
    Ui::ParameterConfig *ui;


protected:
    bool event(QEvent *event);


private:
    QString str_category;
    int tab;
    int row;

    int index_ui;

    bool switch_user_flag = 0;

    Parameters* para;
    ParaSynchronizeThread* pst;

    QString str_temp;
    QString parameter_name;
    QString str_hour;
    QString str_min;
    int parameter_type;

    //set baudrate parameter
    int modeIndex;
    QString modeText;
    QStringList baudrateList;
    int baudrateIndex = 0;
    QString baudrateValue;
    int setValue = 0;
    int setBaudrate = 1;

    //set ip input
    QString ipSegments[4];
    QString ipInputBuffer;
    int currentIpIndex = 0;

    int list_range_set_index;
    QString new_set_value;

    //QString new_name;
    QString mode;

    int is_global;

    QString str_send;

    int is_number = 0;
    int is_readonly = 0;
    int send_flag = 0;
    int ok_step; //indicator at which step of key ok, in step 0, press ok then confirm the para, in the step 1, press ok then sync para.
    int distinguish_flag  = -1;

    int exact_size;
    int exact_index = 0;

    QList<info_prara> readonly_tabs;
    QList<QList <int>> index_list;

public:
    void set_category_string(QString& s) {
        str_category = s;
    }

    void set_row(int r){
        row = r;
    }

public:
    CommAcsSocketThread* comm_acs_thread;
    ActionRegister* action_register;

private:
    WidgetStack* widget_stack;
    //QString strlow,strhigh;
    float low,high;
    int h_low,m_low,h_high,m_high;
    QTableView* tableView;
    int index;
    int count;

signals:
    void change_user();
public slots:
    void show_change_result(int result);
};

#endif // PARAMETER_CONFIG_H
