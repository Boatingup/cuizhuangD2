#include "system_config_page.h"
#include "ui_system_config_page.h"
#include "Resources/key_value.h"
#include "update_progress_page.h"
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTableView>
#include <QRect>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "./Resources/error_code.h"
#include "./Resources/dbg_print.h"
#include "./window/leftpane.h"
#include "show_version.h"
#include "show_runerror.h"
#include "actionconfigure.h"
#include "dialog_showlog.h"
#include "dialog_timer_setup.h"
#include "dialog_usercheckandlogin.h"
#include "dialog_help.h"
#include "fault_display.h"
//#include "driver_chn_config.h"

extern QList <mainPage_uigonfig> menuconfig_list;

SystemConfigPage::SystemConfigPage(int second_level_page, QWidget *parent) :
    QWidget(parent),ui(new Ui::systemConfigPage),second_page_index(second_level_page)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(800, 480);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    this->row = 0;
    this->count = menuconfig_list[second_page_index].nextclass_page.size();

#if 1
    QFile file(":/qss/update_page.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        //LOG(INFO) << ss.toLocal8Bit().data();
        this->setStyleSheet(ss);
        file.close();
    }
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->setMargin(20);
    hlayout->setSpacing(0);

    QRect rec(0, 40, 800, 440);
    hlayout->setGeometry(rec);

    this->tableView = new QTableView();
    tableView->setShowGrid(false); //不显示格子线
    tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
    tableView->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    tableView->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss
    tableView->verticalHeader()->setDefaultSectionSize(40);

    hlayout->addWidget(tableView);

    QStandardItemModel* model = new QStandardItemModel();
    model->setColumnCount(1);
    tableView->setModel(model);
    //设置宽度要在设置了model之后调用
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setVisible(false);
    //表中有被选中的行时，表头不变色
    tableView->horizontalHeader()->setHighlightSections(false);

    QFont font;

    this->index = 0;
    //this->count = 0;

    this->count = menuconfig_list[second_page_index].nextclass_page.size();

    for(int i = 0;i < this->count; i++)
    {
        model->setItem(i, 0, new QStandardItem(menuconfig_list[second_page_index].nextclass_page[i].page_chn));
    }

    for(int row = 0; row < count; row++){
        font = model->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(28);
        model->item(row, 0)->setFont(font);
        model->item(row, 0)->setTextAlignment(Qt::AlignLeft);
    }
    this->tableView->selectRow(this->row);
#else
    for(int i = 0;i < this->count; i++)
    {
        QLabel *label = new QLabel(this);
        label->setGeometry(QRect(0, 80+55*i, 775, 55));
        label->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
        label->setAlignment(Qt::AlignRight);
        m_menu_list.push_back(label);

        QLabel *label_text = new QLabel(this);
        label_text->setGeometry(QRect(120, 80+55*i, 300, 55));
        QFont font;
        font.setFamily(QString::fromUtf8("YouSheBiaoTiHei"));
        font.setPointSize(32);
        label_text->setFont(font);
        label_text->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_text->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_text->setText(menuconfig_list[second_page_index].nextclass_page[i].page_chn);
        m_menu_list.push_back(label_text);
    }
    m_menu_list.at(this->row*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));
    m_menu_list.at(this->row*2 + 1)->move(m_menu_list.at(this->row*2 + 1)->x()-20, m_menu_list.at(this->row*2 + 1)->y());
#endif
    TLOG_INFO("*****hmi:in system_config_page");
}

SystemConfigPage::~SystemConfigPage()
{
    delete ui;
    delete tableView;
    //for(int i = 0;i < this->count*2; i++)
        //delete m_menu_list.at(i);
}


bool SystemConfigPage::event(QEvent *event)
{
    //leftPane* parameter_show;
    QString msg;
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed

        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_NUM_1:{
            pass_enter_testApp += QString::number(1);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_NUM_3:{
            pass_enter_testApp += QString::number(3);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_NUM_6:{
            pass_enter_testApp += QString::number(6);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }

        case KeyValue::KEY_PUSH_PULL:{
            pass_enter_testApp += QString::number(8);
            if(pass_enter_testApp.contains("316318"))
            {
                comm_acs_thread->control_timer->stop();
                char cmd[1024] = {0};
                sprintf(cmd, "killall HMI; /userdata/bin/zdyz_G_hardTest --platform linuxfb & ");
                system(cmd);
            }
            else
                this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }

        case KeyValue::KEY_ARROW_UP:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->row > 0){
                this->tableView->selectRow(--this->row);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->row < (count - 1)){
                this->tableView->selectRow(++this->row);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_MINUS:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:        
        comm_acs_thread->control_timer->stop();
        if(menuconfig_list[7].nextclass_page[this->row].access == 1)
        {
            qDebug() << "****hmi: system_config_page: key ok";
            TLOG_INFO("****hmi: system_config_page: key ok, row = %d", row);
            if(this->row == 0){//健康检查
                QString category = "healthy";
                leftPane* parameter_show = new leftPane(category);
                parameter_show->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = parameter_show;
            } else if (this->row == 1){//时间设置
                Dialog_timer_setup * setupTime = new Dialog_timer_setup(this);
                setupTime->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = setupTime;

            } else if (this->row == 2){//版本信息
                show_version *pversion = new show_version(this);
                pversion->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = pversion;
            } else if (this->row == 3){//错误信息
                QString test = "fault";
                fault_display *fault = new fault_display(test);
                fault->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = fault;
            } else if (this->row == 4){//帮助信息
                Dialog_help *help = new Dialog_help(this);
                help->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = help;
            }/*else if(this->row == 5){//动作配置
                QString test = "test";
                ActionConfigure *action_cfg = new ActionConfigure(test);
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = action_cfg;
                action_cfg->show();
            }*/else if(this->row == 5){//重启
                /*QString test = "test";
                driver_chn_config *driver_config = new driver_chn_config(test);//driver_chn_config::getInstance(test);//new driver_chn_config(test);
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = driver_config;
                driver_config->show();*/
                system("reboot");
            }else if(this->row == 6){//工作面重启
                QJsonObject obj;
                obj[QString("time")] = 1;
                QJsonObject json_obj;
                QJsonArray array ;
                array.append(0);
                json_obj[QString("direction")] = array;
                json_obj[QString("topic")] = "system_reboot";
                json_obj[QString("message")] = obj;
                QJsonDocument jsonDoc(json_obj);
                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                this->comm_acs_thread->m_client->publish(QString("extern_msg"), jsonString.toUtf8().data());

                system("reboot");
            }else if(this->row == 7){//phy复位
                static bool flag = false;
                system("echo 137 > /sys/class/gpio/export");
                system("echo out > /sys/class/gpio/gpio137/direction");
                if(!flag){
                    flag = true;
                    system("echo 0 > /sys/class/gpio/gpio137/value");
                } else {
                    flag = false;
                    system("echo 1 > /sys/class/gpio/gpio137/value");
                }
                //system("echo 0 > /sys/class/gpio/gpio137/value");

                system("echo 137 > /sys/class/gpio/unexport");
            }
        }
        this->comm_acs_thread->cmdManager->lock_ui_off();
        break;
        case KeyValue::KEY_CANCEL:

            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~SystemConfigPage();
            TLOG_INFO("****hmi: system_config_page: key cancel");
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

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




