#ifndef MAINPAGE_H
#define MAINPAGE_H


#include <QWidget>
#include <QProcess>
#include <QVector>
#include <QLabel>

#include "main.h"
#include "../Resources/configure.h"
#include "../Resources/system_state.h"
#include "../Resources/address_set.h"
#include "../Resources/sensor.h"
#include "../Resources/action_register.h"
#include "./threads/update_thread.h"
#include "../Resources/gui_setting.h"
#include "../Resources/mlabel.h"

#include "stoplockpage.h"

#include "../threads/comm_acs_socket_thread.h"
#include "../threads/comm_acs_thread.h"
#include "../threads/miscellaneous_config_thread.h"

#include "widget_stack.h"
#include "controlled_page.h"
#include "net_phone_page.h"
#include "../Resources/parameters.h"
//#include "../Resources/tcpclient.h"
//#include "../Resources/tcpserver.h"
#include "dialog_info.h"
#include "leftpane.h"
#include "parameter_config.h"
#include  "dialog_warrant.h"
#include "port_visualization.h"

namespace Ui {
class mainPage;
}

typedef struct tag_MainPageSensor_t
{
    QLabel *label_name;
    QLabel *label_value;
}MainPageSensor_t;

class mainPage : public QWidget
{
    Q_OBJECT

public:
    enum{
        View_Local_Address      = 0x00000001,
        View_System_State       = 0x00000002,
        View_Shearer_Position   = 0x00000004,
        View_Controlled_Address = 0x00000008,
        View_Pressure           = 0x00000010,
        View_Stroke             = 0x00000020,
        View_Dip_Angle          = 0x00000040,
        View_Height             = 0x00000080,
        View_Action_Name        = 0x00000100,
        View_Temperature        = 0x00000200,
        View_Humidity           = 0x00000400,
        View_Version            = 0x00000800,
        View_System_Date        = 0x00001000,
        View_Follow_Dir         = 0x00002000,
        View_Follow_Step        = 0x00004000,
        View_PITCH_ANGLE        = 0x00008000,
        View_ROLL_ANGLE         = 0x00010000,
        View_EXTERN_SENSOR      = 0x00020000,
        View_SUP_TYPE           = 0x00040000,
        View_DUST               = 0x00080000,
        View_COAL_INFO          = 0x00100000,
        View_All                = 0xFFFFFFFF
    };

    enum coal_para_section{
        inter_rack_delay = 0,//架间延迟
        ending_beam_time,//收尾梁时间
        plug_in_time,//收插板时间
        pause_time1,//停顿时间1
        tail_swing_times,//尾摆次数
        tail_swing_extension_time,//尾摆伸时间
        tail_swing_in_time,//尾摆收时间
        pause_time2,//停顿时间2
        extension_time,//伸插板时间
        tail_beam_extension_time //伸尾梁时间
    };

public:
    //mainPage(QThread* thread_can = nullptr, QThread* thread_uart = nullptr,QWidget *parent = nullptr);
    explicit mainPage(QWidget *parent = nullptr);
    void setDeviceNum(quint8 num){deviceNumber = num;}
    quint8 getDeviceNum(void){return deviceNumber;}
    ~mainPage();
    int checkfileIntegrity(QString file);
    void get_internal_info();
    QString getInfraredSwitch();
    bool event(QEvent *event);
    void enter_controlled_page_nomutex(int page);
    void get_system_rackno();
    void getGroupNeighbourActAditionalList();

    uint8_t calculate_height(void);
private:
    void handle_system_state_normal(QKeyEvent *);
    void Init_Glog();
    void getcurrentUser(QString &user);
    void OutPutVersion();
    void Init_GuiConfig();

    void hide_neg_icons();
    void show_neg_icons();

    void hide_aux_icons();
    void show_aux_icons(int type);

    void hide_date_time_icon();
    void show_date_time_icon();
    void apply_idle_status_style(bool idle_style);

    void auxiliary_menu_show(int index);
    QString get_aux_eng_name(int offset);
    void mainpagesensorinit();
    void update_sensor_ui(QString strname, QString strvalue);
private:
    Ui::mainPage *ui;
    quint8 deviceNumber;
public slots:
    void showInfo(const myInfo &info);
    void timerUpdate(void);
    void statusSigHandler(STATUS_ENUM status);
    void Update_version(int index);

    void set_fault_status(int level);
    void set_net_status(int level);

    void show_memory_coal(int start_end);
    void update_system_state(QString state);
    void update_sensor_data(unsigned int view);
    void update_action_data(unsigned int view);
    void update_config_data(unsigned int view);

    void show_net_phone(int);
    void change_coal_para(QStringList para_list);

    void Showauxiliary();//展示附加动作界面

    void cb_signal_update_input_passwd_interval(int value);

signals:
    void signal_enter_controlled_page(int master_no);
    void signal_exit_controlled_page();

    void signal_action_controlled_page(QString& action_descriptor, QString &gif_info);
    void signal_update_follow_direction(int dir);
    void signal_update_follow_step(QString step);

    void closefornormal();//关闭所有窗口并展示正常
    //void signal_update_coal();//超时刷新记忆放煤的数据
    void signal_inform_master_audio(bool state);
public slots:
    void enter_controlled_page(int page); // 0 for controlled page, 1 for follow page
    void enter_self_push_pull_page(int page, QString trip);
    void exit_controlled_page();
    void enter_menu_page(int result);
    void update_slogan(QString slogan);

    void cb_audio_keep_live();
private:
    int m_index{0};//辅助动作显示下标
    bool m_aux_show_flag{false};//辅助动作显示
    bool m_audio_flag{false};
    int m_aux_type;//辅助动作类型 0-本架 1-临架 2-隔架 3-成组
    int m_aux_num;//辅助动作数量
    int m_aux_page;//辅助动作页数

    int logfile;
    time_t last_login_time{0};

    QString hwflag_str;
    QString hmi_version;

    int main_control_time;
    int goal_position;
    bool stop_flag{false};

    MLabel *m_label_slogan;
    QString m_set_slogan;
    QString mCurrentState;

    QTimer *coal_dead_line;
    QTimer m_audio_live_signal;

    WidgetStack* widget_stack;
    Parameters* para;
    MisCellConfig *mis_config;
    controlled_page* controlled_action_page;
    NetPhonePage* net_phone_page;
    Dialog_info* memory_coal = nullptr;
    leftPane * display_pane = nullptr;
    ParameterConfig *para_config = nullptr;
    Dialog_warrant *dialog_warrant = nullptr;
    SystemState* ss = nullptr;
    AddressSet *address_set;
    ActionRegister* action_register;
    gui_setting *gui_set;
    Sensor *sensor;

    QVector<QLabel *> m_aux_label_action;//附加动作label
    QVector<QLabel *> m_aux_label_sign;//附加动作背景
    QList <QPair <QString,Action_info>> m_action_aditional_list;//附加动作列表

    int m_input_passwd_interval{60};

    MainPageSensor_t mainpageSensorlabel[6]{0};

public:
    CommAcsSocketThread* comm_acs_thread;
    UpdateThread * update_thread;
};

#endif // MAINPAGE_H
