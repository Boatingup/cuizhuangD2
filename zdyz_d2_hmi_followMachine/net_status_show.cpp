#include "net_status_show.h"
#include "ui_net_status_show.h"

#include <QEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <unistd.h>

#include "Resources/key_value.h"
#include "Resources/mqtt_message.h"

net_status_show::net_status_show(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::net_status_show)
{
    ui->setupUi(this);

    comm_acs_thread = CommAcsSocketThread::getInstance();

    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    connect(comm_acs_thread->cmdManager,SIGNAL(singnal_inform_status_changed(int)),
            this, SLOT(cb_get_dev_status(int)));

    m_start_no = comm_acs_thread->cmdManager->m_min_address;
    m_end_no = comm_acs_thread->cmdManager->m_max_address;
    m_route_no = comm_acs_thread->cmdManager->m_route_address;

    setupUI();
    for(int i=0; i<255; i++){
        setStatus(i+1, !comm_acs_thread->cmdManager->m_dev_status[i]);
    }
}

net_status_show::~net_status_show()
{
    delete ui;
}

bool net_status_show::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_AUX_R3:{
            comm_acs_thread->control_timer->stop();
            QJsonObject obj;
            obj["get"] = 1;
            QJsonDocument jsonDoc(obj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->m_client->publish(QString("TOPIC_WILL_MSG"), jsonString.toUtf8().data());
            break;
        }
        case KeyValue::KEY_ARROW_RIGHT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:{
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_UP:{
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_PLUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_MINUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:{
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~net_status_show();
            usleep(200000);
            QJsonObject jsonObj;
            jsonObj["main_page"] = 1;
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
            TLOG_INFO("***hmi: in menu page key cancel. %s %d", __FILE__, __LINE__);
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
