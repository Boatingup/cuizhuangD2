#include "mainpage.h"
#include "ui_mainpage.h"

#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QEvent>
#include <QKeyEvent>
#include <QDateTime>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#include "../Resources/key_value.h"
#include "../Resources/system_state.h"
#include "../Resources/address_set.h"
#include "../Resources/system_state.h"
#include "../Resources/gui_setting.h"

#include "fault_display.h"
#include "../threads/comm_acs_thread.h"
#include "menupage.h"
#include "controlled_action_page.h"
#include "net_phone_page.h"
#include "../Resources/error_code.h"
#include <mutex>

#include <stdio.h>
#include "update_progress_page.h"
//#include "host.h"
#include "open62541.h"
#include <fstream>
#include "auxiliarymenu.h"
#include "dialog_usercheckandlogin.h"
#include "login_in.h"
#include "../Resources/mqtt_message.h"
#include "combination_switch.h"
#include "net_status_show.h"

#define LOGIN_TIME_OUT 60

extern QList <mainPage_uigonfig> menuconfig_list;
extern QString syzk_user;//当前用户
extern QStringList user_get;//配置文件中的用户列表
extern int opc_client_connected;
QString syzk_user;//当前用户
QStringList user_get;//配置文件中的用户列表
extern char ip_client[16];//client ip
extern bool is_large;
//extern guisetting
using namespace std;

extern int g_coal_machine_fence;
int g_coal_machine_fence = -1;
//动作锁
extern std::mutex action_mutex;
//组播音量
extern int GroupVolume;
extern int video_frames;
extern int GroupVolumeDistance;
//升级相关
extern struct  Update_info update_info;
int update_flag_workface;
extern UA_Boolean running_client;
//OPC 动态重连标志位
int opc_client_connected = 0;
extern Gui_config guisetting;
void runOpcClient();

extern QString Category_Index;

extern char up_date_mode[128];
//QProcess *m_proces_bash;
extern bool key_right_healthy_flag;
extern leftPane *sys_parameter_show;
extern leftPane *control_parameter_show;
extern leftPane *follow_parameter_show;
extern leftPane *healthy_parameter_show;
extern leftPane *upgrade_parameter_show;
extern void WriteContrllerStatys(char * sstatus);
extern unsigned int g_stop_lock_info;
extern int g_human_num;
extern QVector<QMap<QString, FAULT_INFO_S>> g_fault;

QString Category_Index;

/*
#define LOG_FILE_PATH_BASE "/others/log/HMI"
#define LOG_FILE_PATH_INFO "/others/log/HMI/INFO"
#define LOG_FILE_PATH_WARN "/others/log/HMI/WARN"
#define LOG_FILE_PATH_ERROR "/others/log/HMI/ERROR"
*/

void mainPage::hide_date_time_icon()
{
    ui->dateEdit->hide();
    ui->timeEdit->hide();
    ui->label_dest->show();
    ui->label_sign_controlled_no->show();
    ui->label_sign_state->show();
}

void mainPage::show_date_time_icon()
{
    ui->dateEdit->show();
    ui->timeEdit->show();
    ui->label_dest->hide();
    ui->label_sign_controlled_no->hide();
    ui->label_sign_state->hide();

    ui->dateEdit->setFocusPolicy(Qt::NoFocus);
    ui->timeEdit->setFocusPolicy(Qt::NoFocus);
}

void mainPage::hide_neg_icons()
{
    ui->label_bg_neg_p1->hide();
    ui->label_bg_neg_p2->hide();
    ui->label_bg_neg_t1->hide();
    ui->label_bg_neg_t2->hide();

    ui->label_sign_neg_p1->hide();
    ui->label_sign_neg_p2->hide();
    ui->label_sign_neg_t1->hide();
    ui->label_sign_neg_t2->hide();

    ui->label_neg_p1->hide();
    ui->label_neg_p2->hide();
    ui->label_neg_t1->hide();
    ui->label_neg_t2->hide();

    ui->label_neg_p1->setText(QString("---"));
    ui->label_neg_p2->setText(QString("---"));
    ui->label_neg_t1->setText(QString("---"));
    ui->label_neg_t2->setText(QString("---"));
}

void mainPage::show_neg_icons()
{
    ui->label_bg_neg_p1->show();
    ui->label_bg_neg_p2->show();
    ui->label_bg_neg_t1->show();
    ui->label_bg_neg_t2->show();

    ui->label_sign_neg_p1->show();
    ui->label_sign_neg_p2->show();
    ui->label_sign_neg_t1->show();
    ui->label_sign_neg_t2->show();

    ui->label_neg_p1->show();
    ui->label_neg_p2->show();
    ui->label_neg_t1->show();
    ui->label_neg_t2->show();
}

void mainPage::hide_aux_icons()
{
    ui->label->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/back_ground.png")));
    ui->label_my_info->hide();
    m_aux_show_flag = false;

    ui->label_L1->hide();
    ui->label_L2->hide();
    ui->label_L3->hide();
    ui->label_R1->hide();
    ui->label_R2->hide();
    ui->label_R3->hide();

    ui->label_action_L1->hide();
    ui->label_action_L2->hide();
    ui->label_action_L3->hide();
    ui->label_action_R1->hide();
    ui->label_action_R2->hide();
    ui->label_action_R3->hide();

    ui->label_action_L1->setText(QString(""));
    ui->label_action_L2->setText(QString(""));
    ui->label_action_L3->setText(QString(""));
    ui->label_action_R1->setText(QString(""));
    ui->label_action_R2->setText(QString(""));
    ui->label_action_R3->setText(QString(""));

    ui->label_support_type->show();
    ui->label_sign_my_number->show();
    ui->label_local_address->show();
    ui->label_back_ground->show();
    ui->label_back_ground_2->show();
    ui->label_sign_controlled_no_2->show();
    ui->label_sign_controlled_no_3->show();

    ui->label_shearer_position->show();
    ui->label_sign_coal_pos->show();
    ui->label_temp->hide();
    ui->label_hum->hide();
    ui->label_sign_controlled_no_8->hide();
    ui->label_sign_controlled_no_9->hide();
    ui->label_back_ground_7->hide();
    ui->label_back_ground_8->hide();
    ui->label_coal_info->show();

    m_label_slogan->show();
}

void mainPage::auxiliary_menu_show(int index)
{
    int basic_pos = index*6;
    int i;
    for(i=0; i<6; i++)
    {
        if(basic_pos+i < m_aux_num){
            m_aux_label_action[i]->setText(m_action_aditional_list.at(basic_pos+i).second.action_name);
            if(i%2 != 0){
                m_aux_label_sign[i]->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r.png")));
            }else{
                m_aux_label_sign[i]->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l.png")));
            }

            m_aux_label_sign[i]->show();
            m_aux_label_action[i]->show();
        } else {
            m_aux_label_sign[i]->hide();
            m_aux_label_action[i]->hide();
        }
    }
}

QString mainPage::get_aux_eng_name(int offset)
{
    int basic_pos = m_index*6;
    if(basic_pos+offset < m_aux_num)
        return m_action_aditional_list.at(basic_pos+offset).second.key_eng_name;
    else
        return QString("none");
}

void mainPage::show_aux_icons(int type)
{
    if(type == 0){
        m_action_aditional_list = action_register->private_action_aditional_list;
    } else if (type ==1){
        m_action_aditional_list = action_register->neighbour_action_aditional_list;
    } else if (type == 2){
        m_action_aditional_list = action_register->interval_action_aditional_list;
    } else if (type == 3){
        m_action_aditional_list = action_register->group_action_aditional_list;
    } else {
        m_action_aditional_list.clear();
    }

    m_aux_num = m_action_aditional_list.size();
    if(m_aux_num == 0)
        return;

    ui->label->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/back_ground_aux.png")));
    ui->label_my_info->show();

    ui->label_support_type->hide();
    ui->label_sign_my_number->hide();
    ui->label_local_address->hide();

    ui->label_shearer_position->hide();
    ui->label_sign_coal_pos->hide();
    ui->label_coal_info->hide();

    m_label_slogan->hide();

    m_aux_page = m_aux_num/6 + ((m_aux_num%6)==0?0:1);

    m_aux_type = type;
    m_aux_show_flag = true;
    m_index = 0;
    auxiliary_menu_show(m_index);

}

void mainPage::Init_GuiConfig()
{
    gui_set->getConfig();
}

void mainPage::get_internal_info()
{
    this->para->Decrypt(guisetting.user_passwd_en,guisetting.user_passwd);
    this->para->Decrypt(guisetting.admin_passwd_en,guisetting.admin_passwd);
    this->para->Decrypt(guisetting.dev_passwd_en,guisetting.dev_passwd);
}

void mainPage::cb_signal_update_input_passwd_interval(int value)
{
    m_input_passwd_interval = value;
}

void mainPage::change_coal_para(QStringList para_list)
{
    QString file = "parameters.xml";
    QString category = "Template";
    QString subblock = "one_key_coal";
    QString item;
    QString value = "real_value";
    int index = 1;

    while(para_list.size() > (index + 1))
    {
            int step = index -1;
            switch (step) {
            case inter_rack_delay:
                item = "ending_beam_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                index++;
                //this->para->update(file,category,subblock,item,value,test);
            break;
            case ending_beam_time :
                item = "plug_in_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
            break;
            case plug_in_time :
                item = "pause_time1";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case pause_time1 :
                item = "tail_swing_times";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case tail_swing_times :
                item = "tail_swing_extension_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->para->update(file,category,subblock,item,value,para_list[index++]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case tail_swing_extension_time :
                item = "tail_swing_in_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case tail_swing_in_time :
                item = "pause_time2";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case pause_time2 :
                item = "extension_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            case extension_time :
                item = "tail_beam_extension_time";
                this->para->update(file,category,subblock,item,value,para_list[++index]);
                //this->cmdManager->para->update("parameters.xml",category,subblock,item,value,cmdline[index++]);
            break;
            default:
            break;
        }
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void mainPage::set_fault_status(int level)
{
    QString fault_desp;
    for(int i=g_fault.size()-1; i>=0; i--)
    {
        for(QMap<QString, FAULT_INFO_S>::const_iterator it = g_fault.at(i).constBegin(); it!=g_fault.at(i).constEnd(); it++)
        {
            fault_desp += it.value().fault_display;
            fault_desp += ";";
        }
    }

    if(level == 1){
        ui->label_fault->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/故障状态1级.png")));
        ui->label_fault->show();
        update_slogan(fault_desp);
    }else if(level == 2){
        ui->label_fault->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/故障状态2级.png")));
        ui->label_fault->show();
        update_slogan(fault_desp);
    }else if(level == 3){
        ui->label_fault->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/故障状态3级.png")));
        ui->label_fault->show();
        update_slogan(fault_desp);
    }else{
        update_slogan(m_set_slogan);
        ui->label_fault->hide();
    }
    return ;
}

void mainPage::set_net_status(int level)
{
    if(level == 1){
        ui->label_wifi->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/wifi已连接.png")));
        ui->label_wifi->show();
    }else if(level == 0){
        ui->label_wifi->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/wifi断开.png")));
        ui->label_wifi->show();
    }
    return ;
}

/*从系统配置文件中获取架号*/
void mainPage::get_system_rackno()
{
    string file = "/userdata/config/SysState";
    ifstream addr_file;
    addr_file.open(file.data());
    string buf;
    string local_id ("option position '");
    unsigned int i;
    char data[32] = {0};
    while(getline(addr_file,buf))
    {
        if(0 == buf.compare(0, 18, local_id, 0, 18))
        {
            i = 18;
            while(buf.at(i) >= '0' && buf.at(i) <= '9')
            {
                data[i-18] = buf.at(i);
                i++;
                if(i == buf.length())break;
            }
            address_set->set_local_address(static_cast<uint8_t>(atoi(data)));
            break;
        }
    }
    addr_file.close();
}

mainPage::mainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainPage)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    ParaSynchronizeThread *pst = ParaSynchronizeThread::getInstance();
    m_label_slogan = new MLabel(this);
    m_label_slogan->setGeometry(12, 323, 776, 35);
    m_label_slogan->set_font_size(18);
    m_label_slogan->raise();
    m_label_slogan->setAlignment(Qt::AlignCenter);
    ui->label_action->hide();
    hide_neg_icons();
    hide_aux_icons();

    if(access("/var/run/vsftpd/empty", F_OK) != 0)
        mkdir("/var/run/vsftpd/empty", 0777);

    this->gui_set = gui_setting::getInstance();
    Init_GuiConfig();//初始化GUI配置

#if 1
    // zlog初始化
    int rc;
    rc = TLOG_INIT("HMI");
    if (!rc) {
        DBG_ERR("zlog init failed\n");
    }
    // 使用zlog的函数接口输出日志信息，替换TLOG_INFO
    TLOG_INFO("mainPage -----\n");
#endif

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->start(1000);

    QFile file(":/qss/mainpage.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in mainpage.cpp\n");
    }else{
        QString styleSheet = file.readAll();
        styleSheet =styleSheet.right(styleSheet.length()-3);
        this->setStyleSheet(styleSheet);
        file.close();
    }

    this->ss = SystemState::getInstance();
    this->address_set = AddressSet::getInstance();
    this->sensor = Sensor::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->update_thread = UpdateThread::getInstance();
    this->para = Parameters::getInstance();
    this->action_register = ActionRegister::getInstance();

    mainpagesensorinit();
    /*根据配置文件更新本架架号*/
    get_system_rackno();
    ui->label_local_address->setText(QString("%3").arg(address_set->get_local_address(), 3, 10, QLatin1Char('0')));

    get_internal_info();
    getcurrentUser(syzk_user);

    QString menu = "accessManagemnet";
    this->para->query_for_menuconfig(menu,menuconfig_list);
    ss->set_system_state(SystemState::Normal);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();

    this->controlled_action_page = nullptr;
    this->net_phone_page = nullptr;
    this->memory_coal = nullptr;

    
    QString category_name = "system";
    QString tab_name = "system_para";
    QString parameter_name = "GroupVolume";
    QString label = "real_value";
    QString str = this->para->query(category_name, tab_name, parameter_name, label);
    GroupVolume = str.toInt();

    parameter_name = "coal_machine_enter";
    g_coal_machine_fence = this->para->query(category_name, tab_name, parameter_name, label).toInt();

    QStringList range_list;
    QString update_index;
    QString update_item;
    QString str_label = "real_value";
    parameter_name = "video_frames";
    range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
    update_index = this->para->query(category_name, tab_name, parameter_name, str_label);

    if(range_list.size() <= 0){
        video_frames = 10;
    }else {
        update_item = range_list[update_index.toInt()];
        video_frames = update_item.toInt();
    }

    parameter_name = "GroupVolumeDistance";
    str = this->para->query(category_name, tab_name, parameter_name, label);
    GroupVolumeDistance = str.toInt();
    if(GroupVolumeDistance == 0)
        GroupVolumeDistance = 1;

    parameter_name = "goal_position";
    str = this->para->query(category_name, tab_name, parameter_name, label);
    goal_position = str.toInt();
    ui->label_shearer_position->setText(QString("%3").arg(goal_position, 3, 10, QLatin1Char('0')));

    parameter_name = "hydraulic_supporter_type";
    range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
    update_index = this->para->query(category_name, tab_name, parameter_name, str_label);
    if(update_index.toInt() > range_list.size()){
        ui->label_sign_my_number->setText(range_list[0]);
    } else {
        ui->label_sign_my_number->setText(range_list[update_index.toInt()]);
    }

    QString category_name_health = "healthy";
    QString tab_name_health = "system_set";
    QString parameter_name_health = "input_passwd_interval";
    str = this->para->query(category_name_health, tab_name_health, parameter_name_health, label);
    if(!str.isNull())
        m_input_passwd_interval = str.toInt();

    QString category_name_follow = "follow_para";
    QString tab_name_follow = "follow_status";
    QString parameter_name_follow = "start_stop_switch";
    QString value = "0";
    str = this->para->query(category_name_follow, tab_name_follow, parameter_name_follow, label);
    if(str.toInt() != 0)
        this->para->update("parameters.xml", category_name_follow, 0, parameter_name_follow, label, value);

    parameter_name_follow = "follow_stage";
    str = this->para->query(category_name_follow, tab_name_follow, parameter_name_follow, label);
    if(str.toInt() != 0)
        this->para->update("parameters.xml", category_name_follow, 0, parameter_name_follow, label, value);

//    category_name_follow = "other_config";
//    tab_name_follow = "company_info";
//    parameter_name_follow = "company_logo";
//    str = this->para->query(category_name_follow, tab_name_follow, parameter_name_follow, label);
//    QFile logo_file(str);
//    if(logo_file.exists())
//    {
//        ui->label_logo_company->clear();
//        ui->label_logo_company->setPixmap(QPixmap(str));
//    }

    category_name_follow = "other_config";
    tab_name_follow = "company_info";
    ui->label_logo_company->clear();
    ui->label_logo_company->setScaledContents(true);
    ui->label_logo_company->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/sany_title2.png")));
    ui->label_logo_company->raise();

    parameter_name_follow = "propaganda_slogan";
    m_set_slogan = this->para->query(category_name_follow, tab_name_follow, parameter_name_follow, label);
    m_label_slogan->setText(m_set_slogan);

    //para->get_calbration_param();

    coal_dead_line = new QTimer();

    connect(pst, &ParaSynchronizeThread::sig_update_slogan,
            this, [this](const QString &text){
        m_set_slogan = text;
        m_label_slogan->setText(m_set_slogan);
    });

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_input_passwd_interval(int)),
            this,
            SLOT(cb_signal_update_input_passwd_interval(int)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(change_mem_coal_para(QStringList)),
            this,
            SLOT(change_coal_para(QStringList)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_system_state(QString)),
            this,
            SLOT(update_system_state(QString)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_sensor_data(unsigned int)),
            this,
            SLOT(update_sensor_data(unsigned int)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_action_data(unsigned int)),
            this,
            SLOT(update_action_data(unsigned int)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_config_data(unsigned int)),
            this,
            SLOT(update_config_data(unsigned int)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(self_push_pull(int, QString)),
            this,
            SLOT(enter_self_push_pull_page(int, QString)));

    connect(this,
            SIGNAL(signal_enter_controlled_page(int)),
            this,
            SLOT(enter_controlled_page(int)));

    connect(this,
            SIGNAL(signal_exit_controlled_page()),
            this,
            SLOT(exit_controlled_page()));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_show_net_phone(int)),
            this,
            SLOT(show_net_phone(int)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_show_memory_coal(int)),
            this,
            SLOT(show_memory_coal(int)));

    connect(update_thread,SIGNAL(from_thread_update(int)),comm_acs_thread,SLOT(closeAllandShowUpdate(int)));
    connect(this,SIGNAL(closefornormal()),comm_acs_thread,SLOT(closeAllandforNormal()));
    connect(comm_acs_thread,SIGNAL(thread_to_update(int)),this,SLOT(Update_version(int)));
    connect(comm_acs_thread->cmdManager,
            SIGNAL(toShowauxiliary()),
            this,
            SLOT(Showauxiliary()));
    connect(comm_acs_thread,
            SIGNAL(inform_fault_status(int)),
            this,
            SLOT(set_fault_status(int)));
    
    connect(comm_acs_thread,
            SIGNAL(inform_net_status(int)),
            this,
            SLOT(set_net_status(int)));

    connect(this, SIGNAL(signal_inform_master_audio(bool)), comm_acs_thread->cmdManager, SLOT(cb_master_audio_state(bool)));

    connect(&m_audio_live_signal, SIGNAL(timeout()), this, SLOT(cb_audio_keep_live()));
    m_audio_live_signal.stop();
    system("amixer cset numid=2,iface=MIXER,name='Capture MIC Path' 'Main Mic'");

    update_thread->start();
    this->ui->label_pedestal_dip->hide();

    category_name = "system";
    int tab = 0;
    parameter_name = "controlTime";
    label ="real_value";

    QString str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    if(str_main_control_time != ""){
        this->main_control_time = static_cast<int>(1000 * str_main_control_time.toFloat());
    }else{
        this->main_control_time = 10000; //10s as default time back to main page.
    }

    this->comm_acs_thread->main_control_time = this->main_control_time;

    parameter_name = "controlAlertTime";
    str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    this->comm_acs_thread->m_alter_control_time = this->comm_acs_thread->main_control_time;
    if(str_main_control_time != ""){
        this->comm_acs_thread->m_alter_control_time = static_cast<int>(1000 * str_main_control_time.toFloat());
    }

    parameter_name = "groupAlertTime";
    str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    this->comm_acs_thread->m_group_control_time = this->comm_acs_thread->main_control_time;
    if(str_main_control_time != ""){
        this->comm_acs_thread->m_group_control_time = static_cast<int>(1000 * str_main_control_time.toFloat());
    }
    ui->label_fault->hide();

    m_aux_label_action.push_back(ui->label_action_L1);
    m_aux_label_action.push_back(ui->label_action_R1);
    m_aux_label_action.push_back(ui->label_action_L2);
    m_aux_label_action.push_back(ui->label_action_R2);
    m_aux_label_action.push_back(ui->label_action_L3);
    m_aux_label_action.push_back(ui->label_action_R3);

    m_aux_label_sign.push_back(ui->label_L1);
    m_aux_label_sign.push_back(ui->label_R1);
    m_aux_label_sign.push_back(ui->label_L2);
    m_aux_label_sign.push_back(ui->label_R2);
    m_aux_label_sign.push_back(ui->label_L3);
    m_aux_label_sign.push_back(ui->label_R3);

    ui->dateEdit->setDisplayFormat("yy/MM/dd ddd");
    ui->timeEdit->setDisplayFormat("HH:mm:ss");

    ui->dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

    show_date_time_icon();
}

#include <QDomDocument>
#include <QtXml>

void mainPage::mainpagesensorinit()
{
    mainpageSensorlabel[0].label_name = ui->label_sign_controlled_no_2;
    mainpageSensorlabel[0].label_value = ui->label_pressure_1;

    mainpageSensorlabel[1].label_name = ui->label_sign_controlled_no_3;
    mainpageSensorlabel[1].label_value = ui->label_pressure_2;

    mainpageSensorlabel[2].label_name = ui->label_sign_controlled_no_4;
    mainpageSensorlabel[2].label_value = ui->label_pitch_angle;

    mainpageSensorlabel[3].label_name = ui->label_sign_controlled_no_5;
    mainpageSensorlabel[3].label_value = ui->label_roll_angle;

    mainpageSensorlabel[4].label_name = ui->label_sign_controlled_no_6;
    mainpageSensorlabel[4].label_value = ui->label_height;

    mainpageSensorlabel[5].label_name = ui->label_sign_controlled_no_7;
    mainpageSensorlabel[5].label_value = ui->label_stroke;

    QFile file("/userdata/config/hmi_files/sensorconfig.xml");

    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open parameter file failed. %s %d", __FILE__, __LINE__);
        return;
    }
    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        file.close();
        TLOG_INFO("Dom set content failed. %s %d", __FILE__, __LINE__);
        return;
    }
    file.close();

    QDomElement root=doc.documentElement();

    QDomNodeList list = root.elementsByTagName("sensorname");

    for (int i = 0; i < list.count(); i++)
    {
        QDomElement element = list.at(i).toElement();

        if(i < 6)
        {
            mainpageSensorlabel[i].label_name->setText(element.firstChild().nodeValue());
        }
    }
}

void mainPage::Showauxiliary()
{
    is_large = false;
    if(this->comm_acs_thread->cmdManager->is_show_auxiliary == false)
    {
        AuxiliaryMenu *auxiliary_menu = new AuxiliaryMenu(this);
        auxiliary_menu->show();
        this->widget_stack->push(this);
        this->comm_acs_thread->cmdManager->window = auxiliary_menu;
        this->comm_acs_thread->cmdManager->is_show_auxiliary = true;
        this->comm_acs_thread->cmdManager->lock_ui_off();
    } else {
        this->comm_acs_thread->cmdManager->lock_ui_off();

    }
}

void mainPage::Update_version(int index)
{
    TLOG_INFO("in update version");
}

void mainPage::getcurrentUser(QString &user)
{
    if(user.isNull()){
    }
    QString category_name = "system";
    QString tab_name = "system_para";
    QString parameter_name = "current_user";
    QString label = "real_value";
    QString Access = "access";

    QStringList range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
    QString index = this->para->query(category_name, tab_name, parameter_name, label);
    if(range_list.size() == 0 || index == "")
    {
        TLOG_INFO("no user found !");
        syzk_user = "user";
    } else {
        syzk_user = range_list[index.toInt()];
    }
}

QString mainPage::getInfraredSwitch()
{
    QString category_name("set_para");
    QString parameter_name("uwb_set_distance");
    QString label("real_value");
    return(this->para->query(category_name,2,parameter_name,label));
}

mainPage::~mainPage()
{
    delete coal_dead_line;
    delete ui;
}

//void mainPage::Init_Glog()
//{
//    if(access(LOG_FILE_PATH_BASE, F_OK) != 0)
//        mkdir(LOG_FILE_PATH_BASE, 0777);
//    if(access(LOG_FILE_PATH_INFO, F_OK) != 0)
//        mkdir(LOG_FILE_PATH_INFO, 0777);
//    if(access(LOG_FILE_PATH_WARN, F_OK) != 0)
//        mkdir(LOG_FILE_PATH_WARN, 0777);
//    if(access(LOG_FILE_PATH_ERROR, F_OK) != 0)
//        mkdir(LOG_FILE_PATH_ERROR, 0777);

//    google::InitGoogleLogging("hmi");
//    google::SetLogDestination(google::GLOG_INFO, "/others/log/HMI/INFO/HMI_INFO_");
//    google::SetLogDestination(google::GLOG_WARNING, "/others/log/HMI/WARN/HMI_WARNING_");
//    google::SetLogDestination(google::GLOG_ERROR, "/others/log/HMI/ERROR/HMI_ERR_");
//    google::SetLogFilenameExtension("log_");
//    FLAGS_logbufsecs = 0;  // Set log output speed(s)
//    FLAGS_max_log_size = 10;  // Set max log file size
//    FLAGS_stop_logging_if_full_disk = true;  // If disk is full
//}

void mainPage::update_system_state(QString state){
    if(mCurrentState == state)
    {
        return;
    }
    mCurrentState = state;
    QFont font;
    font.setPointSize(40);
    //ui->label_system_state->setFont(font);
    ui->label_sign_state->clear();
    ui->label_sign_state->show();
    hide_neg_icons();
    hide_aux_icons();
    ui->label_action->hide();
    ui->label_system_state->setFont(font);
    ui->label_human_num->hide();
    if(state == "Initial"){
        ui->label_system_state->setText("初始化");
        this->ss->set_system_state(SystemState::Initial);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/初始化.png")));
        show_date_time_icon();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Normal") {
        if(this->comm_acs_thread->cmdManager->window == this->memory_coal)
            return ;
        memset(sensor->extern_pressure, 0, sizeof(sensor->extern_pressure));
        memset(sensor->extern_trip, 0, sizeof(sensor->extern_trip));
        this->comm_acs_thread->closeAllwindow();
        ui->label_system_state->setText("空闲");
        this->ss->set_system_state(SystemState::Normal);
        TLOG_INFO(" rest to narmal ");
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/\344\270\273\347\225\214\351\235\242/\347\251\272\351\227\262.png")));
        ui->label_sign_controlled_no->setText("受控架号");
        ui->label_dest->setText("000");
        ui->label_warning->hide();
        show_date_time_icon();
        emit this->signal_exit_controlled_page();
    }else if (state == "Controlled"){
        ui->label_system_state->setText("受控");
        this->ss->set_system_state(SystemState::Controlled);
        emit this->signal_enter_controlled_page(0); //0 for controlled page
    }else if (state == "RemoteControlled") {
        ui->label_system_state->setText("遥控");
        this->ss->set_system_state(SystemState::RemoteControlled);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/遥控.png")));
        hide_date_time_icon();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Neighbour"){
        ui->label_system_state->setText("邻架");
        this->ss->set_system_state(SystemState::Neighbour);
        hide_date_time_icon();
        ui->label_sign_state->hide();
        show_neg_icons();
        show_aux_icons(1);
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Interval") {
        ui->label_system_state->setText("隔架");
        this->ss->set_system_state(SystemState::Interval);
        hide_date_time_icon();
        ui->label_sign_state->hide();
        show_neg_icons();
        show_aux_icons(2);
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Group"){
        ui->label_system_state->setText("成组");
        this->ss->set_system_state(SystemState::Group);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/成组.png")));
        hide_date_time_icon();
        show_aux_icons(3);
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Stop") {
        this->comm_acs_thread->closeAllwindow();
        ui->label_system_state->setText("急停");
        this->ss->set_system_state(SystemState::Stop);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/急停.png")));
        ui->label_sign_controlled_no->setText("急停架号");
        ui->label_dest->setText(QString("%3").arg(g_stop_lock_info, 3, 10, QLatin1Char('0')));
        hide_date_time_icon();
        ui->label_warning->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/预警.png")));
        ui->label_warning->setScaledContents(true);
        ui->label_warning->show();
        ui->label_warning->raise();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Lock"){
        this->comm_acs_thread->closeAllwindow();
        ui->label_system_state->setText("闭锁");
        this->ss->set_system_state(SystemState::Lock);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/闭锁.png")));
        ui->label_sign_controlled_no->setText("闭锁架号");
        ui->label_dest->setText(QString("%3").arg(g_stop_lock_info, 3, 10, QLatin1Char('0')));
        hide_date_time_icon();
        ui->label_warning->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/预警.png")));
        ui->label_warning->setScaledContents(true);
        ui->label_warning->show();
        ui->label_warning->raise();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "HumanLock"){
        this->comm_acs_thread->closeAllwindow();
        font.setPointSize(24);
        ui->label_system_state->setFont(font);
        ui->label_system_state->setText("人员闭锁");
        this->ss->set_system_state(SystemState::Lock);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/闭锁.png")));
        ui->label_sign_controlled_no->setText("闭锁架号");
        ui->label_dest->setText(QString("%3").arg(g_stop_lock_info, 3, 10, QLatin1Char('0')));
        if(g_human_num > 0){
            ui->label_human_num->setText(QString::number(g_human_num)+QString("人"));
            ui->label_human_num->show();
        }
        hide_date_time_icon();
        ui->label_warning->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/预警.png")));
        ui->label_warning->setScaledContents(true);
        ui->label_warning->show();
        ui->label_warning->raise();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Update") {
        ui->label_system_state->setText("升级");
        this->ss->set_system_state(SystemState::Update);
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if (state == "Address"){
        ui->label_system_state->setText("编址");
        this->ss->set_system_state(SystemState::Address);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/编址.png")));
        hide_date_time_icon();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }else if(state == "Follow"){
        this->comm_acs_thread->closeAllwindow();
        ui->label_system_state->setText("跟机");
        this->ss->set_system_state(SystemState::Follow);
        //ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/跟机.png")));
        //hide_date_time_icon();
        show_date_time_icon();
        ui->label_warning->hide();
        this->comm_acs_thread->cmdManager->lock_ui_off();
        emit this->signal_exit_controlled_page();
    }else if (state == "Coal_Put"){
        ui->label_system_state->setText("记忆");
        this->ss->set_system_state(SystemState::Coal_Put);
        ui->label_sign_state->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/记忆学习.png")));
        hide_date_time_icon();
    }else if (state == "LocalReal") {
        ui->label_system_state->setText("自控");
        this->ss->set_system_state(SystemState::LocalReal);
        ui->dateEdit->hide();
        ui->timeEdit->hide();
        ui->label_action->setText("预警");
        ui->label_action->show();
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }
}

void mainPage::update_sensor_ui(QString strname, QString strvalue)
{
    for(int i = 0; i < 6; i++)
    {
        //TLOG_INFO("**********%d*********%s %s", i, strname.toUtf8().data(), mainpageSensorlabel[i].label_name->text().toUtf8().data());
        if(mainpageSensorlabel[i].label_name->text() == strname)
        {
            //TLOG_INFO("%s***************%s %s", mainpageSensorlabel[i].label_value->text().toUtf8().data(), strname.toUtf8().data(), strvalue.toUtf8().data());
            if(mainpageSensorlabel[i].label_value->text() != strvalue)
            {
                //TLOG_INFO("**********%d*********%s", i, strvalue.toUtf8().data());
                mainpageSensorlabel[i].label_value->setText(strvalue);
                break;
            }
        }
    }
}

void mainPage::update_sensor_data(unsigned int view){

    if(view & View_COAL_INFO){
        QString display;
        if(sensor->coal_mechine_direction == 1){
            display = QString("上行");
        } else if(sensor->coal_mechine_direction == 2){
            display = QString("下行");
        } else{
            display = QString("停止");
        }
        display += QString(":") + QString::number(sensor->coal_mechine_speed);

        ui->label_sign_coal_pos->setText(display);
    }

    if(view & View_EXTERN_SENSOR){
        if(sensor->extern_pressure[1] < 0.0f || sensor->extern_pressure[1] > 60.0f){
            ui->label_neg_p1->setText(QString("---"));
        }else{
            ui->label_neg_p1->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->extern_pressure[1]), 'f', 1)));
        }

        if(sensor->extern_pressure[2] < 0.0f || sensor->extern_pressure[2] > 60.0f){
            ui->label_neg_p2->setText(QString("---"));
        }else{
            ui->label_neg_p2->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->extern_pressure[2]), 'f', 1)));
        }

        if(sensor->extern_trip[1] < 0 || sensor->extern_trip[1] > 5000)
        {
            ui->label_neg_t1->setText(QString("---"));
        }else {
            ui->label_neg_t1->setText(QString("%1").arg(sensor->extern_trip[1]));
        }

        if(sensor->extern_trip[2] < 0 || sensor->extern_trip[2] > 5000)
        {
            ui->label_neg_t2->setText(QString("---"));
        }else {
            ui->label_neg_t2->setText(QString("%1").arg(sensor->extern_trip[2]));
        }

    }

    if(view & View_Pressure){

        //TLOG_INFO("%f %f %f %f", sensor->pressure[0], sensor->pressure[1], sensor->pressure[2], sensor->pressure[3]);
        if(sensor->pressure[1] < 0.0f || sensor->pressure[1] > 60.0f){ //initialized with -1.0
            //ui->label_pressure_1->setText(QString("---"));
            update_sensor_ui(mainpageSensorlabel[0].label_name->text(), QString("---"));
        }else{
            //ui->label_pressure_1->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[1]), 'f', 1)));
            update_sensor_ui(mainpageSensorlabel[0].label_name->text(), QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[1]), 'f', 1)));
        }

        if(sensor->pressure[2] < 0.0f || sensor->pressure[2] > 60.0f){
            //ui->label_pressure_2->setText(QString("---"));
            update_sensor_ui(mainpageSensorlabel[1].label_name->text(), QString("---"));
        }else{
            //ui->label_pressure_2->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[2]), 'f', 1)));
            update_sensor_ui(mainpageSensorlabel[1].label_name->text(), QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[2]), 'f', 1)));
        }

        if(sensor->pressure[3] < 0.0f || sensor->pressure[3] > 60.0f){
            update_sensor_ui(QString("压力3"), QString("---"));
        }else{
            update_sensor_ui(QString("压力3"), QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[2]), 'f', 1)));
        }

        if(sensor->pressure[4] < 0.0f || sensor->pressure[4] > 60.0f){
            update_sensor_ui(QString("压力4"), QString("---"));
        }else{
            update_sensor_ui(QString("压力4"), QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[2]), 'f', 1)));
        }
    }

    if(view & View_Stroke){
        if(sensor->stroke[1] < 0 || sensor->stroke[1] > 5000)
        {
            update_sensor_ui(QString("行程"), QString("---"));
        }
        else {
            update_sensor_ui(QString("行程"), QString("%1").arg(sensor->stroke[1]));
        }

        if(sensor->stroke[2] < 0 || sensor->stroke[2] > 5000)
        {
            update_sensor_ui(QString("行程2"), QString("---"));
        }
        else {
            update_sensor_ui(QString("行程2"), QString("%1").arg(sensor->stroke[1]));
        }

        if(sensor->stroke[3] < 0 || sensor->stroke[3] > 5000)
        {
            update_sensor_ui(QString("行程3"), QString("---"));
        }
        else {
            update_sensor_ui(QString("行程3"), QString("%1").arg(sensor->stroke[1]));
        }

        if(sensor->stroke[4] < 0 || sensor->stroke[4] > 5000)
        {
            update_sensor_ui(QString("行程4"), QString("---"));
        }
        else {
            update_sensor_ui(QString("行程4"), QString("%1").arg(sensor->stroke[1]));
        }
    }

    if(view & View_Temperature){
        if(sensor->temperature < -20.0f || sensor->temperature > 125.0f){
            update_sensor_ui(QString("温度"), QString("---"));
        }
        else {
            update_sensor_ui(QString("温度"),QString("%1").arg(QString::number(static_cast<double>(sensor->temperature), 'f', 1)));
        }
    }

    if(view & View_Humidity){
        if(sensor->humidity < 0.0f || sensor->humidity > 100.0f){
            update_sensor_ui(QString("湿度"), QString("---"));
        }
        else {
            update_sensor_ui(QString("湿度"),QString("%1").arg(QString::number(static_cast<double>(sensor->humidity), 'f', 1)));
        }
    }

    if(view & View_Height){
        if(sensor->height == 0){
            if(sensor->calc_hight_flag)
                update_sensor_ui(QString("高度"), QString("---"));
        } else {
            if(!sensor->calc_hight_flag)
                sensor->calc_hight_flag = true;
            update_sensor_ui(QString("高度"), QString("%1").arg(QString::number(sensor->height)));
        }
    }

    if(view & View_PITCH_ANGLE){
        if((sensor->pitch_angle < -180.0)||(sensor->pitch_angle > 180.0))
            update_sensor_ui(QString("俯仰角"), QString("---"));
        else
            update_sensor_ui(QString("俯仰角"), QString("%1").arg(QString::number(static_cast<double>(sensor->pitch_angle))));

    }

    if(view & View_ROLL_ANGLE){
        if((sensor->roll_angle < -180.0)||(sensor->roll_angle > 180.0))
            update_sensor_ui(QString("翻滚角"), QString("---"));
        else
            update_sensor_ui(QString("翻滚角"), QString("%1").arg(QString::number(static_cast<double>(sensor->roll_angle))));
    }

    if(view & View_Shearer_Position ){
        ui->label_shearer_position->setText(QString("%3").arg(sensor->shearer_position, 3, 10, QLatin1Char('0')));
    }

    if(view & View_SUP_TYPE){
        QString category_name = "system";
        QString tab_name = "system_para";
        QString label = "real_value";
        QString parameter_name = "hydraulic_supporter_type";
        QStringList range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
        QString update_index = this->para->query(category_name, tab_name, parameter_name, label);
        if(update_index.toInt() > range_list.size()){
            ui->label_sign_my_number->setText(range_list[0]);
        } else {
            ui->label_sign_my_number->setText(range_list[update_index.toInt()]);
        }
    }

    this->comm_acs_thread->cmdManager->lock_ui_off();
}


void mainPage::update_action_data(unsigned int view){
    QString action_name;
    if(view & View_Action_Name){
        QStringList id_list;
        if(!this->address_set->action_id_str.isEmpty()){
            id_list = this->address_set->action_id_str.split("&");
        } else {
            id_list.clear();
        }

        if((this->ss->get_system_state() == SystemState::Follow) && id_list.size() == 0){
            emit signal_exit_controlled_page();
        } else {

            if(id_list.size() == 0){
                action_name = "预警";
            } else {
                this->action_register->Find_action(id_list, action_name);
            }
            QString gif_info;
            if(id_list.size() == 2)
                this->action_register->find_gif_by_name(id_list[0], gif_info);

            if(this->ss->get_system_state() == SystemState::Follow){
                TLOG_INFO("view: 33333 %d", view);
                this->enter_controlled_page_nomutex(1);
            } else if(this->ss->get_system_state() == SystemState::Self_pull) {
                QString temp_name = action_name;
                action_name = "本架" + temp_name;
            }
            this->address_set->action_id_str.clear();
            emit signal_action_controlled_page(action_name, gif_info);
            if(this->ss->get_system_state() == SystemState::LocalReal)
                ui->label_action->setText(action_name);
        }
    }

    if(view & View_Follow_Dir){
        emit signal_update_follow_direction(this->address_set->follow_dir);

        QString display;// = "跟机-";
        if(this->address_set->follow_dir==2){
            display += "下行";
        }else if (this->address_set->follow_dir==1) {
            display += "上行";
        }else {
            display += "停止";
        }

        if(this->address_set->follow_step == "初始"){
            display += "U0";
        }else if (this->address_set->follow_step == "上行1") {
            display += "U1";
        }else if (this->address_set->follow_step == "上行2") {
            display += "U2";
        }else if (this->address_set->follow_step == "上行3") {
            display += "U3";
        }else if (this->address_set->follow_step == "上行4") {
            display += "U4";
        }else if (this->address_set->follow_step == "下行1") {
            display += "D1";
        }else if (this->address_set->follow_step == "下行2") {
            display += "D2";
        }else if (this->address_set->follow_step == "下行3") {
            display += "D3";
        }else if (this->address_set->follow_step == "下行4") {
            display += "D4";
        }

        ui->label_system_state->setText(display);
    }
    if(view & View_Follow_Step){
        if(this->ss->get_system_state() == SystemState::Follow){
            emit signal_update_follow_step(this->address_set->follow_step);
            QString display;//= "跟机-";
            if(this->address_set->follow_dir==2){
                display += "下行";
            }else if (this->address_set->follow_dir==1) {
                display += "上行";
            }else {
                display += "停止";
            }

            if(this->address_set->follow_step == "初始"){
                display += "U0";
            }else if (this->address_set->follow_step == "上行1") {
                display += "U1";
            }else if (this->address_set->follow_step == "上行2") {
                display += "U2";
            }else if (this->address_set->follow_step == "上行3") {
                display += "U3";
            }else if (this->address_set->follow_step == "上行4") {
                display += "U4";
            }else if (this->address_set->follow_step == "下行1") {
                display += "D1";
            }else if (this->address_set->follow_step == "下行2") {
                display += "D2";
            }else if (this->address_set->follow_step == "下行3") {
                display += "D3";
            }else if (this->address_set->follow_step == "下行4") {
                display += "D4";
            }

            ui->label_system_state->setText(display);
        }
    }

    if(view & View_Controlled_Address){
        if(ss->get_system_state() == SystemState::Group){
            if(address_set->get_action_exe_direction() == AddressSet::ACTION_EXE_DIR_LEFT_TO_RIGHT )
            {
                ui->label_dest->setText(QString("%1>>%2")
                                        .arg(address_set->get_destination_address_start())
                                        .arg(address_set->get_destination_address_end()));
            }
            else if(address_set->get_action_exe_direction() == AddressSet::ACTION_EXE_DIR_RIGHT_TO_LEFT)
            {
                ui->label_dest->setText(QString("%1<<%2")
                                        .arg(address_set->get_destination_address_start())
                                        .arg(address_set->get_destination_address_end()));
            }
            else {
                ui->label_dest->setText(QString("[%1-%2]")
                                        .arg(address_set->get_destination_address_start())
                                        .arg(address_set->get_destination_address_end()));
            }
            ui->label_sign_controlled_no->setText(QString("受控架号"));

        }else if(ss->get_system_state() == SystemState::Stop) {
            ui->label_dest->setText(QString("%3").arg(this->address_set->get_stop_last(), 3, 10, QLatin1Char('0')));
            ui->label_sign_controlled_no->setText(QString("急停架号"));

        }else if(ss->get_system_state() == SystemState::Lock) {
            ui->label_dest->setText(QString("%3").arg(address_set->get_lock_last(), 3, 10, QLatin1Char('0')));
            ui->label_sign_controlled_no->setText(QString("闭锁架号"));
        }else{
            ui->label_dest->setText(QString("%3").arg(address_set->get_destination_address(), 3, 10, QLatin1Char('0')));
            ui->label_sign_controlled_no->setText(QString("受控架号"));
        }
        TLOG_INFO("view: 10000 %d", view);
        this->comm_acs_thread->cmdManager->lock_ui_off();
    }
}

void mainPage::update_config_data(unsigned int view){
    if(view & View_Local_Address){
        ui->label_local_address->setText(QString("%3").arg(address_set->get_local_address(), 3, 10, QLatin1Char('0')));
    }
    if(view & View_System_Date){
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();

}

void mainPage::enter_controlled_page_nomutex(int page){
    if(this->controlled_action_page == nullptr){
        this->controlled_action_page = new controlled_page(this);
        this->controlled_action_page->show();
        this->widget_stack->push(this);
        this->comm_acs_thread->cmdManager->window = controlled_action_page;
    }
}

void mainPage::enter_controlled_page(int page){
    if(this->controlled_action_page == nullptr){
        this->controlled_action_page = new controlled_page(this);
        this->controlled_action_page->show();
        //this->widget_stack->push(this);
        this->widget_stack->push(this->comm_acs_thread->cmdManager->window);
        this->comm_acs_thread->cmdManager->window = controlled_action_page;
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void mainPage::enter_self_push_pull_page(int page, QString trip){
    if(this->controlled_action_page == nullptr){
        this->controlled_action_page = new controlled_page(this);
        this->controlled_action_page->show();
        //this->widget_stack->push(this);
        this->widget_stack->push(this->comm_acs_thread->cmdManager->window);
        this->comm_acs_thread->cmdManager->window = controlled_action_page;
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void mainPage::exit_controlled_page(){
#if 0
    if(this->controlled_action_page != nullptr){
        this->controlled_action_page->close();
        this->controlled_action_page->~controlled_page();
        this->controlled_action_page = nullptr;
        LOG(INFO) << "exit controlled page";
    }
#endif
    this->controlled_action_page = nullptr;
    emit closefornormal();
}


void mainPage::show_memory_coal(int start_end){
    if(start_end == 1){ //start net phone page
        if(this->memory_coal != nullptr)
            return ;
        this->comm_acs_thread->cmdManager->sleep_timer.restart();
        this->comm_acs_thread->cmdManager->coal_flag = 0;
        QString category = "control_para";
        QString subblock = "coal_drawing";
        QString label = "learn_interval";
        QString item = "real_value";
        QString value;
        value = this->para->query(category,subblock,label,item);
        coal_dead_line->stop();
        connect(coal_dead_line, &QTimer::timeout, [=](){
            if(this->comm_acs_thread->cmdManager->coal_flag == 0) {
                DBG_ERR("qtimer timeout after 2s\n");
                this->comm_acs_thread->cmdManager->coal_flag = 1;
                this->comm_acs_thread->cmdManager->RecordMemory_coal();
                emit this->comm_acs_thread->cmdManager->signal_show_memory_coal(2);
            } else {
                this->memory_coal = nullptr;
            }
        });
        coal_dead_line->setSingleShot(true);
        coal_dead_line->start(static_cast<int>(1000 * value.toFloat()));
        this->memory_coal = new Dialog_info("记忆模板","开始记忆模板",QString::number(static_cast<int>(value.toFloat())) + "秒后结束记录",static_cast<int>(value.toFloat()));
        this->memory_coal->show();
        this->widget_stack->push(this);
        this->comm_acs_thread->cmdManager->window = this->memory_coal;
        //通知不在主界面
        QJsonObject jsonObj;
        jsonObj["main_page"] = 0;
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());

    } else if(start_end == 0){
        if(this->comm_acs_thread->cmdManager->coal_flag == 1)
        {
            this->memory_coal->close();
            this->comm_acs_thread->cmdManager->list_template.clear();
            this->ss->set_system_state(SystemState::Normal);
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->memory_coal->~Dialog_info();
            this->memory_coal = nullptr;
            QJsonObject jsonObj;
            jsonObj["main_page"] = 1;
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
        }
    } else {
        if(this->comm_acs_thread->cmdManager->coal_flag == 1)
        {
            this->memory_coal->close();
            this->comm_acs_thread->cmdManager->list_template.clear();
            this->ss->set_system_state(SystemState::Normal);
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->memory_coal->~Dialog_info();
            this->memory_coal = nullptr;
            QJsonObject jsonObj;
            jsonObj["main_page"] = 1;
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
        }
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void mainPage::show_net_phone(int start_end){
    if(this->net_phone_page == nullptr){
        if(start_end == 1){ //start net phone page
            this->net_phone_page = new NetPhonePage();
            this->net_phone_page->show();
        }else{ //close net phone page
        }
    }else{
        if(start_end == 0){//close net phone page
            this->net_phone_page->close();
            this->net_phone_page->~NetPhonePage();
            this->net_phone_page = nullptr;
        }else if (start_end > 100) {
            this->net_phone_page->set_occuried(start_end - 100);
        }
    }
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void mainPage::showInfo(const myInfo &info)
{
    float test = info.fPress1;
    if(test < info.fPress1){}
}

/*处理时间显示槽函数*/
void mainPage::timerUpdate()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    ui->dateEdit->setDate(current_date_time.date());
    ui->timeEdit->setTime(current_date_time.time());
    current_date_time.date().dayOfWeek();
}

/*处理设备状态槽函数*/
void mainPage::statusSigHandler(STATUS_ENUM status)
{
    if(status == IDEL){}
    return ;
}

bool mainPage::event(QEvent *event)
{
    if((ss != nullptr) && (this->address_set != nullptr)){
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        SystemState::System_state_type ss_type = ss->get_system_state();

        switch(ss_type){

        case SystemState::Normal:
        case SystemState::Follow:
            handle_system_state_normal(key_event);
            break;
        case SystemState::Human_lock:/*人员闭锁双击取消*/
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            TLOG_INFO("not in normal state: %d", ss_type);
            handle_system_state_normal(key_event);
            break;
        }
    }
    return QWidget::event(event);
}

void mainPage::update_slogan(QString slogan)
{
    m_label_slogan->setText(slogan);
    return ;
}

void mainPage::cb_audio_keep_live()
{
    if(!m_audio_flag){
        m_audio_live_signal.stop();
        return;
    }

    QJsonObject audio_obj, child_obj, extern_obj;
    QJsonArray des;
    des.append(0);

    child_obj["promoter"] = address_set->get_local_address();
    child_obj["state"] = QString("keeplive");

    audio_obj["audio"] = child_obj;
    child_obj["volum"] = GroupVolume;
    child_obj["interval"] = GroupVolumeDistance;

    extern_obj["destination"] = des;
    extern_obj["topic"] = QString("broad_cast");
    extern_obj["message"] = audio_obj;

    QJsonDocument jsonDoc(extern_obj);
    comm_acs_thread->m_client->publish(QString(TOPIC_EXTERN_MSG), jsonDoc.toJson());
}

void mainPage::enter_menu_page(int result)
{
    TLOG_WARN("it wants to enter menu %d", result);
//    cout << result <<  " ++++++++++++ result" << endl;
    menuPage *p ;
    if(result == 0)
    {
        QJsonObject jsonObj;
        jsonObj["main_page"] = 1;
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
        return;
    }

    last_login_time = time(nullptr);
    p = new menuPage();
    p->show();
    this->widget_stack->push(this);
    this->comm_acs_thread->cmdManager->window = p;
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

static void *wait_thread(void *param)
{
    pthread_detach(pthread_self());
    pid_t *pid_thread = static_cast<pid_t *>(param);
    waitpid(*pid_thread, nullptr, 0);
    free(param);
    return nullptr;
}

void mainPage::handle_system_state_normal(QKeyEvent *key_event){

    menuPage *p ;
    int offset = 0;
    switch(key_event->type()){
    case KeyValue::KEY_PRESSED:    // key pressed
    {
        switch(key_event->key()){
        case KeyValue::KEY_AUX_L1:{
            if(m_aux_show_flag){
                offset = 0;
                ui->label_L1->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_L2:{
            if(m_aux_show_flag){
                offset = 2;
                ui->label_L2->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_L3:{
            if(m_aux_show_flag){
                offset = 4;
                ui->label_L3->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R1:{
            if(m_aux_show_flag){
                offset = 1;
                ui->label_R1->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R2:{
            if(m_aux_show_flag){
                offset = 3;
                ui->label_R2->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R3:{
            if(m_aux_show_flag){
                offset = 5;
                ui->label_R3->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r_s.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("down");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            } else if (m_audio_flag == false && this->comm_acs_thread->cmdManager->m_audio_flag == false && g_coal_machine_fence == -1){
#if 1
                m_audio_flag = true;
                emit signal_inform_master_audio(m_audio_flag);
                pid_t pid = fork();
                if (pid < 0) {
                    TLOG_ERROR("StartBroadServerCmd fork process error");
                    fflush(stdout);
                }else if (pid == 0) {
                     execlp("/userdata/bin/alsa_rtsp_server","alsa_rtsp_server", "hw:0,0", NULL);
                     exit(-1);
                }else {
                    pthread_t wait_pid;
                    pid_t *thread_pid = static_cast<pid_t *>(malloc(sizeof(pid_t)));
                    *thread_pid = pid;
                    pthread_create(&wait_pid, nullptr, wait_thread, thread_pid);
                }

                QJsonObject audio_obj, child_obj, extern_obj;
                QJsonArray des;
                des.append(0);

                child_obj["promoter"] = address_set->get_local_address();
                child_obj["state"] = QString("start");
                child_obj["volum"] = GroupVolume;
                child_obj["interval"] = GroupVolumeDistance;

                audio_obj["audio"] = child_obj;

                extern_obj["destination"] = des;
                extern_obj["topic"] = QString("broad_cast");
                extern_obj["message"] = audio_obj;

                QJsonDocument jsonDoc(extern_obj);
                comm_acs_thread->m_client->publish(QString(TOPIC_EXTERN_MSG), jsonDoc.toJson());

                m_audio_live_signal.start(5000);

                ui->label_system_state->setText("喊话中");
#endif
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
    }
    break;

    case KeyValue::KEY_RELEASED:  // key released
    {
        switch(key_event->key()){
        case KeyValue::KEY_OK:{
#if 1
            if((m_input_passwd_interval != 911) && (time(nullptr) - last_login_time > m_input_passwd_interval))
            {
                login_in *p_window = new login_in("admin");
                connect(p_window,SIGNAL(change_result(int)),this,SLOT(enter_menu_page(int)));
                p_window->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = p_window;
                this->comm_acs_thread->cmdManager->lock_ui_off();
                QJsonObject jsonObj;
                jsonObj["main_page"] = 0;
                QJsonDocument jsonDoc(jsonObj);
                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
                break;
            }
            last_login_time = time(nullptr);
#endif
             cout << "key_event->type(): " << key_event->type() << " key_event->key(): " << key_event->key() << endl;
            p = new menuPage();
            p->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = p;
            QJsonObject jsonObj;
            jsonObj["main_page"] = 0;
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
}
        case KeyValue::KEY_CANCEL:
            if(m_aux_show_flag)
            {
                hide_neg_icons();
                hide_aux_icons();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_PLUS:
            cout << "KEY_ARROW_PLUS" << "++" << endl;
            if(m_aux_show_flag == false){
                show_aux_icons(0);
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_AUX_L1:{
            cout << "KEY_AUX_L1" << "++" << endl;
            if(m_aux_show_flag)
            {
                offset = 0;
                ui->label_L1->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            } else {
#if 0
                combination_switch *p_window = new combination_switch();
                p_window->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = p_window;
#endif
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_L2:{

            if(m_aux_show_flag)
            {
                offset = 2;
                ui->label_L2->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_L3:{
            if(m_aux_show_flag)
            {
                offset = 4;
                ui->label_L3->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_l.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R1:{
            if(m_aux_show_flag)
            {
                offset = 1;
                ui->label_R1->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R2:{
            if(m_aux_show_flag)
            {
                offset = 3;
                ui->label_R2->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            }else{
                net_status_show *net = new net_status_show();
                net->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = net;
                QJsonObject jsonObj;
                jsonObj["main_page"] = 0;
                QJsonDocument jsonDoc(jsonObj);
                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
                this->comm_acs_thread->cmdManager->lock_ui_off();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_AUX_R3:{
            if(m_aux_show_flag)
            {
                offset = 5;
                ui->label_R3->setPixmap(QPixmap(QString::fromUtf8(":/new/main_page/image/主界面/aux_button_bg_r.png")));
                QJsonObject json, json_child;
                json_child[get_aux_eng_name(offset)] = QString("up");
                json["aux"] = json_child;
                QJsonDocument jsonDoc(json);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonDoc.toJson());
            } else if (m_audio_flag){
#if 1
                m_audio_flag = false;
                emit signal_inform_master_audio(m_audio_flag);

                system("kill -9 $(pidof alsa_rtsp_server)");

                QJsonObject audio_obj, child_obj, extern_obj;
                QJsonArray des;
                des.append(0);

                child_obj["promoter"] = address_set->get_local_address();
                child_obj["state"] = QString("stop");
                child_obj["volum"] = GroupVolume;
                child_obj["interval"] = GroupVolumeDistance;

                audio_obj["audio"] = child_obj;

                extern_obj["destination"] = des;
                extern_obj["topic"] = QString("broad_cast");
                extern_obj["message"] = audio_obj;

                QJsonDocument jsonDoc(extern_obj);
                comm_acs_thread->m_client->publish(QString(TOPIC_EXTERN_MSG), jsonDoc.toJson());
                m_audio_live_signal.stop();

                ui->label_system_state->setText("空闲");
#endif
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_LEFT:
        {
            cout << "KEY_ARROW_LEFT" << "++" << endl;
            if(m_aux_show_flag == false){
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }

            if(m_index > 0)
                --m_index;
            else
                m_index = m_aux_page-1;
            auxiliary_menu_show(m_index);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_RIGHT:
        {
            cout << "KEY_ARROW_RIGHT" << "++" << endl;
            if(m_aux_show_flag == false){
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }


            if(m_index < (m_aux_page-1))
                ++m_index;
            else
                m_index = 0;
            auxiliary_menu_show(m_index);

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_DOWN:
        {
            cout << "KEY_ARROW_DOWN" << "++" << endl;
#if 0
            key_right_healthy_flag = true;
            QString category = "system";
            //Category_Index = "system";
            display_pane = new leftPane(category);
            display_pane->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = display_pane;
            msg = QString("config -m 0");
            this->comm_acs_thread->send_socket_msg_to_acs(msg);
#endif
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        default:
            TLOG_INFO("invalid key:%d", key_event->key());
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
    }
    break;

    default: break;
    }
}


