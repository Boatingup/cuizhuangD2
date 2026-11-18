#include "show_version.h"
#include "ui_show_version.h"
#include "./Resources/key_value.h"
#include "threads/comm_acs_thread.h"
#include <QEvent>
#include <QKeyEvent>
#include "unistd.h"


show_version::show_version(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::show_version)
{
    ui->setupUi(this);
    this->para = Parameters::getInstance();
    comm_acs_thread = CommAcsSocketThread::getInstance();
    widget_stack = WidgetStack::getInstance();

    QString kernel_version = get_kernel_version();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    ui->label_hmi_version->setText(get_hmi_version());
    ui->label_acs_version->setText(comm_acs_thread->cmdManager->m_acs_version.isNull()?"V0.0.5.240627":comm_acs_thread->cmdManager->m_acs_version);
    ui->label_keyboard_version->setText(comm_acs_thread->cmdManager->m_key_version);
    //ui->label_kernel_version->setText(kernel_version.isNull()?"4.19.193":kernel_version);
    ui->label_update_version->setText(comm_acs_thread->cmdManager->m_update_version.isNull()?"V1.0.2.240711":comm_acs_thread->cmdManager->m_update_version);
    ui->label_data_version->setText(comm_acs_thread->cmdManager->m_data_version.isNull()?"V0.0.1.240519":comm_acs_thread->cmdManager->m_data_version);
    ui->label_opc_version->setText(comm_acs_thread->cmdManager->m_opc_version.isNull()?"V0.0.1.240710":comm_acs_thread->cmdManager->m_opc_version);
    ui->label_mqtt_version->setText(comm_acs_thread->cmdManager->m_mqtt_version.isNull()?"V1.0.1.240705":comm_acs_thread->cmdManager->m_mqtt_version);
    ui->label_health_version->setText(comm_acs_thread->cmdManager->m_health_version.isNull()?"V0.0.2.240706":comm_acs_thread->cmdManager->m_health_version);
    QString pcb_version;
    if(access("/sys/class/i2c-dev/i2c-4", F_OK) == 0){
        pcb_version = "V1.0.1";
    } else {
        pcb_version = "V1.0.2";
    }
    ui->label_kernel_version->setText(pcb_version);
}

static int exec_sys_cmd(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=nullptr)
    {
        while(fgets(buf_ps, 1024, ptr)!=nullptr)
        {
           strcat(result, buf_ps);
           if(strlen(result)>1024)
               break;
        }
        pclose(ptr);
        ptr = nullptr;
        return 0;
    }
    else
    {
        printf("popen %s error\n", ps);
        return 1;
    }
}

QString show_version::get_hmi_version()
{
    QString version;

    version = HMI_VERSION;
    return version;
}

QString show_version::get_kernel_version()
{
    QString version;
    char result[2048] = {0};
#if 0
    const char *cmd = "uname -r";
#else
    const char *cmd = "cat /proc/sys/kernel/version | awk '{print $8 \"/\" $4 \"/\" $5 \" \" $6}'";
#endif
    if(0 == exec_sys_cmd(cmd, result))
        version = result;

    return version;
}

show_version::~show_version()
{
    delete ui;
}

bool show_version::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~show_version();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_RIGHT:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_PLUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_MINUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
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


