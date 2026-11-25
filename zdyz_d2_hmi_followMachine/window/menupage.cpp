#include "menupage.h"
#include "ui_menupage.h"
#include <QAction>
#include "leftpane.h"
#include <QFile>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <unistd.h>
//#include "host.h"
#include "../Resources/key_value.h"

#include "../window/mainpage.h"

#include "./window/mainpage.h"
#include "./window/menupage.h"
#include "./window/leftpane.h"

#include "../threads/comm_acs_thread.h"
#include "../manual_address.h"
#include "system_config_page.h"
#include "../Resources/error_code.h"


#include "update_progress_page.h"
#include "../Resources/guiconfig.h"
#include "dialog_usercheckandlogin.h"
#include "passwd_set.h"
#include "update_config_page.h"

#include "./Resources/mqtt_message.h"

extern struct Update_info update_info;
extern int update_flag_workface;
extern QList <mainPage_uigonfig> menuconfig_list;

extern QString Category_Index;
extern char ip_client[16];//client ip
extern bool updating;
extern bool pc_update_flag;
extern bool net_phone_page;
extern QString syzk_user;//当前用户
extern Gui_config   guisetting;
extern leftPane *sys_parameter_show;
extern leftPane *control_parameter_show;
extern leftPane *follow_parameter_show;
extern leftPane *sysconfig_parameter_show;

leftPane *sys_parameter_show = nullptr;
leftPane *control_parameter_show = nullptr;
leftPane *follow_parameter_show = nullptr;
leftPane *sysconfig_parameter_show = nullptr;

menuPage::menuPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menuPage)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    //this->setAttribute(Qt::WA_DeleteOnClose);
    QFile file(":/qss/menupage.qss");
    if(!file.open(QFile::ReadOnly)){
        TLOG_INFO("open qss file failed in menuPage().");
    }else{
        QString styleSheet = file.readAll();
        this->setStyleSheet(styleSheet);
        file.close();
    }

    ui->label_system->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/系统参数.png"));
    label_index = 1; // in the system parameter interface

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();

    QString category_name("system");
    int tab = 0;
    QString parameter_name("controlTime");
    QString label("real_value");

    QString str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    if(str_main_control_time != ""){
        this->main_control_time = 1000 * str_main_control_time.toFloat();
    }else{
        this->main_control_time = 1000000; //10s as default time back to main page.
    }

    this->comm_acs_thread->main_control_time = this->main_control_time;

    parameter_name = "controlAlertTime";
    str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    this->comm_acs_thread->m_alter_control_time = this->comm_acs_thread->main_control_time;
    if(str_main_control_time != ""){
        this->comm_acs_thread->m_alter_control_time = 1000 * str_main_control_time.toFloat();
    }

    parameter_name = "groupAlertTime";
    str_main_control_time = this->para->query(category_name, tab, parameter_name, label);
    this->comm_acs_thread->m_group_control_time = this->comm_acs_thread->main_control_time;
    if(str_main_control_time != ""){
        this->comm_acs_thread->m_group_control_time = 1000 * str_main_control_time.toFloat();
    }

    this->timer_back_to_mainpage = new QTimer();
    connect(timer_back_to_mainpage, SIGNAL(timeout()), this, SLOT(timer_closecb()));

    timer_back_to_mainpage->setSingleShot(true);

    TLOG_INFO("hmi:main_control_time = %d", main_control_time);

    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
}

menuPage::~menuPage()
{
    delete ui;
    delete timer_back_to_mainpage;
}

void menuPage::timer_closecb(){
    QString msg;
    //this->timer_back_to_mainpage->start(main_control_time);
        //qDebug() << this->widget_stack->size();
    TLOG_INFO("%d\n", this->widget_stack->size());
        this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();

        TLOG_INFO("in timer_cb back to main page.");

        this->close();
        this->~menuPage();

        usleep(200000);
        QJsonObject jsonObj;
        jsonObj["main_page"] = 1;
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());

}

void menuPage::refresh(int old_index, int new_index) const {
    switch (old_index) {
    case 0:
        ui->label_default->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/默认界面.png"));
        break;
    case 1:
        ui->label_system->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/系统参数.png"));
        break;
    case 2:
        ui->label_group->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/成组参数.png"));
        break;
    case 3:
        ui->label_follow->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/跟机参数.png"));
        break;
    case 4:
        ui->label_address->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/编址功能.png"));
        break;
    case 5:
        ui->label_update->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/升级同步.png"));
        break;
    case 6:
        ui->label_password->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/密码设置.png"));
        break;
    case 7:
        ui->label_time->setPixmap(QPixmap(":/new/menu_page/菜单界面/默认状态/系统设置.png"));
        break;
    default:
        break;
    }

    switch (new_index) {
    case 0:
        ui->label_default->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/默认界面.png"));
        break;
    case 1:
        ui->label_system->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/系统参数.png"));
        break;
    case 2:
        ui->label_group->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/成组参数.png"));
        break;
    case 3:
        ui->label_follow->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/跟机参数.png"));
        break;
    case 4:
        ui->label_address->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/编址功能.png"));
        break;
    case 5:
        ui->label_update->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/升级同步.png"));
        break;
    case 6:
        ui->label_password->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/密码设置.png"));
        break;
    case 7:
        ui->label_time->setPixmap(QPixmap(":/new/menu_page/菜单界面/选中状态/系统设置.png"));
        break;
    default:
        break;
    }

}

bool menuPage::event(QEvent *event)
{
    int old;
    //abot update
    QString category_name = "system";
    QString tab_name = "system_para";
    QString parameter_name = "workface_setup";
    QString label = "real_value";
    QMap <QString,int> map_items;
    QStringList range_list;
    QString update_index;
    QString update_index_method;
    QString update_item;

    QString parameter_name_method = "update_method";

    //Note that this category must be a argument to pass
    QString category;

    leftPane* parameter_show;
    ManualAddress *ma;
    QWidget *widget;
    passwd_set *p_changed_passwd;
    QString msg;
    bool no_use = 0;

    //int k;
    //char buf[128] = {0};
    //char buf2[128] = {0};

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            old = label_index;
            label_index++;
            if(label_index > 7){ // at this time,there are totally 8 logos in the menu page.
                label_index = 0;
            }
            refresh(old, label_index);
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            old = label_index;
            label_index--;
            if(label_index < 0){
                label_index = 7;
            }

            refresh(old, label_index);
            //this->timer_back_to_mainpage->start(main_control_time);
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            old = label_index;
            label_index += 4;
            if(label_index > 7){ // at this time,there are totally 8 logos in the menu page.
                label_index -= 8;
            }

            refresh(old, label_index);
            //this->timer_back_to_mainpage->start(main_control_time);
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            old = label_index;
            label_index -= 4;
            if(label_index < 0){
                label_index += 8;
            }

            refresh(old, label_index);

            //this->timer_back_to_mainpage->start(main_control_time);
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
            this->timer_back_to_mainpage->stop();
            if(menuconfig_list.size() <= label_index)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            if(menuconfig_list[label_index].access == 1)
            {
                switch(label_index){
                case 0: //back to default window (main window)
                {
                    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                    this->close();
                    this->~menuPage();
                    QJsonObject jsonObj;
                    jsonObj["main_page"] = 1;
                    QJsonDocument jsonDoc(jsonObj);
                    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
                    break;
                }
                case 1:
                    category = "system";
                    parameter_show = new leftPane(category);
                    //Category_Index = "system";
                    parameter_show->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = parameter_show;
                    break;
                case 2:
                    category = "control_para";
                    parameter_show = new leftPane(category);
                    //Category_Index = "control_para";
                    parameter_show->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = parameter_show;
                    break;
                case 3:
                    category = "follow_para";
                    parameter_show = new leftPane(category);
                    //Category_Index = "follow_para";
                    parameter_show->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = parameter_show;
                    break;
                case 4:
                    ma = new ManualAddress();
                    ma->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = ma;
                    break;

                case 5:
                    widget = new UpdateConfigPage(label_index);
                    widget->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = widget;
                    break;
                case 6:
                    p_changed_passwd = new passwd_set(syzk_user);//new Dialog_UserCheckAndLogin(0,syzk_user,&no_use);
                    p_changed_passwd->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = p_changed_passwd;
                    break;
                case 7:
                    widget = new SystemConfigPage(label_index);
                    widget->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = widget;
                    break;
                default:
                    break;
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:{
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            TLOG_INFO("***hmi: in menu page key cancel. %s %d", __FILE__, __LINE__);
            this->close();
            this->~menuPage();
            TLOG_INFO("***hmi: in menu page key cancel. %s %d", __FILE__, __LINE__);
            usleep(200000);
            QJsonObject jsonObj;
            jsonObj["main_page"] = 1;
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
            TLOG_INFO("***hmi: in menu page key cancel. %s %d", __FILE__, __LINE__);
            //this->comm_acs_thread->cmdManager->lock_ui_off();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;

    default:
        break;
    }

    return QWidget::event(event);
}

