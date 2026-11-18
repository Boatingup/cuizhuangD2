#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <vector>
#include <string>

#include <QDebug>
#include <QObject>

#include "../Resources/system_state.h"
#include "../Resources/sensor.h"
#include "../Resources/address_set.h"
#include "../Resources/action_register.h"
#include "../Resources/parameters.h"
#include "../widget_stack.h"
#include <mutex>
#include <QSemaphore>
#include <QMap>

using namespace std;

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

struct error_msg
{
       QString msg_error;
       QString current_time;
};

typedef struct _fault_info
{
    time_t fault_time;/*故障发送到HMI的时间*/
    QString fault_display;
    QString fault_diagnosis;
    QString fault_occured_time;/*故障产生的时间*/
    int index;
}FAULT_INFO_S;

typedef struct _fault_param
{
    int index;
    int pri;
    QString dignosis;
    QString descriptor;
}FAULT_PARAM_S;

class command;


//this class managers the command type classes in which handling the specific command line
class CmdManager : public QObject{
    Q_OBJECT
public:
    CmdManager();               //register command lines to the manager
    ~CmdManager();
public:
    void split_command(QString &str_cmd);
    void parse_command();
    void parse_command(QStringList cmdline);
    int findIndexByName(QString name,int &index);
    int findSensorIndexByName(QString name,int &index);
    int findWirelessByName(QString name,int &index);

    //void Log_current_queue();
    void lock_ui_off();
    void lock_ui_on();
    bool IsInUrGent();
    int StateCheck(QStringList cmdline);//检查下发按键和状态是否匹配  来自ACS 的误报
    int numberof_connects_systemdefault();
    void RecordMemory_coal();//记忆放煤函数
    void json_format_switch(QString type, const QByteArray &json_data);
    void insert_msg(QString &msg);

signals:

    void signal_process_update(double cur, double total);
    void signal_status_update(QString status);
    void signal_file_set(QString name);

    void signal_show_stop_lock_page(QString stop_lock, QString number);
    void signal_update_system_state(QString state);
    void signal_update_sensor_data(unsigned int view);
    void signal_update_action_data(unsigned int view);
    void signal_update_config_data(unsigned int view);

    void signal_update_ad_data(unsigned int index);

    void signal_update_progress(int progress);

    void signal_show_net_phone(int start_end); // start =1, end = 0
    void signal_show_memory_coal(int start_end);

    void signal_update_system_fault(QString str,QString info);
    void signal_update_acs_version(QString str);

    void singnal_to_socket(QString str);
    void signal_update_fail();

    void self_push_pull(int index, QString trip);

    void close_self_push();

    void receive_share_data(QString str,int len,int sd);

    void update_addata(int index, QString data);

    void toShowauxiliary();
    void checkToquit();
    void checkToVerfy();
    void change_mem_coal_para(QStringList);
    void signal_update_input_passwd_interval(int);

    void singnal_send_hmi_version(const QByteArray msg);
    void singnal_inform_status_changed(int dev_no);
public:
    QList<QString> m_cmd_buf;
    QStringList cmdline;
    QMap<QString, command*> command_map;
    //QString msgreceived;
    QString current_step; //当前记录的采煤步骤
public:
    bool m_dev_status[255] = {false};
    int m_route_address{50};
    int m_max_address{255};
    int m_min_address{1};
    QSemaphore *m_semaphore;
    QMutex *m_mutex;
    ActionRegister* action_register;
    SystemState* ss = nullptr;
    AddressSet* address_set;
    Parameters *para;
    Sensor* sensor;
    int AD[16] = {0};
    float ad6 = 0.0;
    float ad13 = 0.0;

    QString system_default;
    QString acs_version;
    QString hmi_version;
    QString stop_lock_page_name;
    QStringList msg_queue;
    QStringList last_msg;
    QList <error_msg> last_msg_error;
    QList <Template_sheet> list_template;//模板保存
    QElapsedTimer coal_timer; //用于记忆放煤的计时
    QElapsedTimer sleep_timer; //用于记忆放煤的计时

    bool is_show_auxiliary = false;
    bool is_self_action_update = false;
    bool is_self_parameter_update = false;
    bool ok_or_not = false;

    QString m_acs_version;
    QString m_key_version;
    QString m_data_version;
    QString m_opc_version;
    QString m_mqtt_version;
    QString m_health_version;
    QString m_kernel_version;
    QString m_update_version;

    QString m_sbc_1_version;
    QString m_sbc_2_version;
    QString m_signal_version;
    QString m_coal_board_version;

    bool m_rtsp_server_start_flag{false};
public:
    QWidget* window;
    WidgetStack* widget_stack;

public:
    //human lock switch,added for human lock function
    int human_lock_switch; //
    int self_push = 0;
    int human_lock_status = 0;

    bool coal_flag = false;

    QSemaphore *Msg_Sem = nullptr;
    QSemaphore *Pro_Sem = nullptr;
    QMutex *ui_lock;

    bool m_audio_flag{false};
    bool m_master_audio_state{false};
    int m_audio_count{8000};

    QTimer m_timer_count;
public slots:
    void cb_keep_live_count();
    void cb_master_audio_state(bool state);
};


//interface to specific class cmd_xxx, each presents a command line
class command
{
public:
    command(QString cmd_name, CmdManager* pcm);
    virtual ~command();
public:
    virtual void func(const QStringList& cmdline) = 0;  //the key point, this function is a interface
    //virtual void func(const QJsonObject& json) = 0;

public:
    const QString name;
protected:
    CmdManager* cmdManager;
};


class cmd_key : public command{
public:
    cmd_key(CmdManager* cm) : command("key", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_state : public command{
public:
    cmd_state(CmdManager* cm) : command("state", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_select : public command{
public:
    cmd_select(CmdManager* cm) : command("select", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_sensor : public command{
public:
    cmd_sensor(CmdManager* cm) : command("sensor", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_parameter : public command{
public:
    cmd_parameter(CmdManager* cm) : command("parameter", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_action : public command{
public:
    cmd_action(CmdManager* cm) : command("action", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_stop: public command{
public:
    cmd_stop(CmdManager* cm) : command("stop", cm){}
    void func(const QStringList& cmdline);
   // void func(const QJsonObject& json){}
};

class cmd_lock : public command{
public:
    cmd_lock(CmdManager* cm) : command("lock", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_config : public command{
public:
    cmd_config(CmdManager* cm) : command("config", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};


class cmd_addata : public command{
public:
    cmd_addata(CmdManager* cm) : command("addata", cm){}
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
};

class cmd_system_fault : public command{
public:
    cmd_system_fault(CmdManager* cm) : command("system_fault", cm){
        m_driver_chn_config.clear();
        QString file_name = "/userdata/config/hmi_files/fault_diagnosis.xml";
        QString driver_file_name = "/userdata/config/hmi_files/driver_chn_config.xml";
        load_fault_xml(file_name);
        load_driver_xml(driver_file_name);
    }
    void func(const QStringList& cmdline);
    //void func(const QJsonObject& json){}
private:
    QDomDocument fault_xml;
    bool sub_str_to_map(QString input, char sign, QMap<QString, QString> &output);
    bool load_fault_xml(QString& file_path);
    bool load_driver_xml(QString& file_path);
    void fault_handle(QString fault_data);

    QVector<QString> m_driver_chn_config;
};


#endif
