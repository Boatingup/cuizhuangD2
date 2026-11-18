#include "dialog_info.h"
#include "ui_dialog_info.h"
#include <QKeyEvent>
#include <QEvent>

Dialog_info::Dialog_info(QString title,QString content,QString info,float time,QWidget *parent) :
    QDialog(parent),ui(new Ui::Dialog_info),m_title(title),m_content(content),m_info(info),m_delay(time)
{
    ui->setupUi(this);
    GetStylePath();
    this->timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_cb()));
    this->timer->start(1000);
    comm_acs_thread->control_timer->stop();
}


void Dialog_info::doRecord()
{
    if(this->comm_acs_thread->cmdManager->action_register->coal_map[this->comm_acs_thread->cmdManager->current_step].is_start_timer == true)
    {
        int ms = static_cast<int>(this->comm_acs_thread->cmdManager->coal_timer.elapsed());
        Template_sheet sheet;
        sheet.id = this->comm_acs_thread->cmdManager->current_step.toInt();
        float time_interval = static_cast<float>(static_cast<float>(ms) / 1000);
        sheet.time = QString::number(static_cast<double>(time_interval),'f',1);
        sheet.name = this->comm_acs_thread->cmdManager->action_register->coal_map.value(this->comm_acs_thread->cmdManager->current_step).action_eng;
        this->comm_acs_thread->cmdManager->list_template.push_back(sheet);
    }
    else
    {
        int ms = static_cast<int>(this->comm_acs_thread->cmdManager->sleep_timer.elapsed());
        Template_sheet sheet;
        sheet.id = 255;
        float time_interval = static_cast<float>(static_cast<float>(ms) / 1000);
        sheet.time = QString::number(static_cast<double>(time_interval),'f',1);
        sheet.name = "sleep";
        this->comm_acs_thread->cmdManager->list_template.push_back(sheet);
    }
}

void Dialog_info::timer_cb()
{
    ui->label_info->setText(QString::number(static_cast<double>(--m_delay)) + "秒后自动保存! " + "按确认保存，按取消退出");
    if(this->comm_acs_thread->cmdManager->coal_flag == 1)
    {
        ui->label_content->setText("已记录模板");
        doRecord();
        emit this->comm_acs_thread->cmdManager->signal_show_memory_coal(0);
    }
}

void Dialog_info::setFontNumber(QLabel *label,int num)
{
    QFont ft;
    ft.setPointSize(num);
    label->setFont(ft);
}

void Dialog_info::setFontColor(QLabel *label,QColor color)
{
    QPalette pa;
    pa.setColor(QPalette::WindowText,color);
    label->setPalette(pa);
}

int Dialog_info::GetStylePath()
{
    QFile file(":/qss/info.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }

    ui->label_title->setText(m_title);
    ui->label_content->setText(m_content);
    ui->label_info->setText(m_info);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    return 0;
}

Dialog_info::~Dialog_info()
{
    delete this->timer;
    delete ui;
}

bool Dialog_info::event(QEvent *event)
{
    QString msg;
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            doRecord();
            this->comm_acs_thread->cmdManager->RecordMemory_coal();
            //LOG(WARNING) << "config -c";
            //msg = QString("config -c"); // notify that the parameter has been changed
            this->comm_acs_thread->send_socket_msg_to_acs(msg);
            this->comm_acs_thread->cmdManager->coal_flag = 1;
            TLOG_INFO("save ");
            emit this->comm_acs_thread->cmdManager->signal_show_memory_coal(0);
            break;

        case KeyValue::KEY_CANCEL:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->coal_flag = 1;
            TLOG_INFO("cancel ");
            emit this->comm_acs_thread->cmdManager->signal_show_memory_coal(0);
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
