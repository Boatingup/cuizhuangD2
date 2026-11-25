#include "comm_acs_socket_thread.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libgen.h>
#include <linux/tcp.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sys/prctl.h>
#include "./Resources/dbg_print.h"
#include "show_version.h"
#include "./Resources/mqtt_message.h"
#include "para_synchronize_thread.h"

CommAcsSocketThread* CommAcsSocketThread::com_acs_socket_thread = nullptr;

extern int update_flag_workface;

CommAcsSocketThread::CommAcsSocketThread()
{
    pthread_mutex_init(&m_socked_mutex, nullptr);
    this->m_main_page = nullptr;
    this->cmdManager = new CmdManager();
    this->widget_stack = WidgetStack::getInstance();
    this->control_timer = new QTimer(this);
    last_run_time = time(nullptr);
    connect(this->control_timer,SIGNAL(timeout()),this,SLOT(closeAllfortimer_up()));

    connect(cmdManager, SIGNAL(singnal_send_hmi_version(QByteArray)), this, SLOT(cb_send_hmi_version(QByteArray)));

    mqtt_init();
}

void CommAcsSocketThread::cb_mqtt_connected()
{
    DBG_ERR("cb_mqtt_connected\n");
    for(QMap<QString, command*>::const_iterator it = cmdManager->command_map.constBegin(); it != cmdManager->command_map.constEnd(); it++)
    {
        m_client->subscribe(it.key());
    }

    QJsonDocument jsonDoc;

    QJsonObject obj;

    //m_client->subscribe(QString(TOPIC_EXTERN_MSG));
    m_client->subscribe(QString(TOPIC_PARAMETER_EXTERN_SET));
    m_client->subscribe(QString(TOPIC_PARAMETER_EXTERN_V2));
    m_client->subscribe(QString(TOPIC_SCREEN));
    m_client->subscribe(QString(TOPIC_UPDATE));
    m_client->subscribe(QString(TOPIC_REBOOT));
    m_client->subscribe(QString(TOPIC_BROAD_CAST));
    m_client->subscribe(QString(TOPIC_PORT_STATUS));
    m_client->subscribe(QString(TOPIC_MQTT_PROXY_STATUS));
    m_client->subscribe(QString(TOPIC_WILL_MSG));

    obj.insert("get", QJsonValue::Null);
    jsonDoc =  QJsonDocument(obj);

    //QString msg1 = "{\"get\":3}";
    m_client->publish(QString(TOPIC_CONFIG), jsonDoc.toJson());
    m_client->publish(QString(TOPIC_PARAMETER_SET), jsonDoc.toJson());
    m_client->publish(QString(TOPIC_CONFIG), jsonDoc.toJson());
    m_client->publish(QString(TOPIC_STATE), jsonDoc.toJson());
    m_client->publish(QString(TOPIC_SENSOR), jsonDoc.toJson());

    obj.insert("get", 1);
    jsonDoc =  QJsonDocument(obj);

    m_client->publish(QString(TOPIC_FALUT), jsonDoc.toJson());
    m_client->publish(QString(TOPIC_WILL_MSG), jsonDoc.toJson());

    obj.insert("status", 3);
    jsonDoc =  QJsonDocument(obj);

    m_client->publish(QString(TOPIC_MQTT_PROXY_STATUS), jsonDoc.toJson());

    QJsonObject json1;
    json1["online"] = true;
    QJsonDocument jsonDoc2(json1);
    m_client->publish(QString(MQTT_WILL_TOPIC), jsonDoc2.toJson(), 0, true);
}

void CommAcsSocketThread::cb_mqtt_disconnected()
{
    for(QMap<QString, command*>::const_iterator it = cmdManager->command_map.constBegin(); it != cmdManager->command_map.constEnd(); it++)
    {
        m_client->unsubscribe(it.key());
    }
    m_client->unsubscribe(QString(TOPIC_EXTERN_MSG));
}

void CommAcsSocketThread::cb_mqtt_state_changed(QMqttClient::ClientState state)
{

}

void CommAcsSocketThread::cb_mqtt_message_recv(const QByteArray &message, const QMqttTopicName &topic)
{
    if(topic.name().compare(QString("sensor")))
    {
        TLOG_INFO(" recv topic:%s msg:%s\n", topic.name().toUtf8().data(), message.toStdString().c_str());
    }

    this->cmdManager->json_format_switch(topic.name(), message);
//    if(topic.name() == TOPIC_PARAMETER_EXTERN_SET){ //外部数据
//        ParaSynchronizeThread::getInstance()->parse_msg(message);
//    } else {
//    }
}

void CommAcsSocketThread::cb_mqtt_ping_recv()
{

}

void CommAcsSocketThread::cb_mqtt_error(QMqttClient::ClientError error)
{
    switch (error) {
       case QMqttClient::NoError:
           TLOG_ERROR("No error");
           break;
       case QMqttClient::InvalidProtocolVersion:
           TLOG_ERROR("Unacceptable protocol version error");
           break;
       case QMqttClient::IdRejected:
           TLOG_ERROR("Invalid client error");
           break;
       default:
           TLOG_ERROR("%d", error);
           break;
       }
}

void CommAcsSocketThread::cb_send_hmi_version(const QByteArray msg)
{
    send_socket_msg_to_acs(QString(TOPIC_CONFIG), msg);
}

void CommAcsSocketThread::mqtt_init()
{
    m_client = new QMqttClient(this);

    connect(m_client, &QMqttClient::connected, this, &CommAcsSocketThread::cb_mqtt_connected);
    connect(m_client, &QMqttClient::disconnected, this, &CommAcsSocketThread::cb_mqtt_disconnected);
    connect(m_client, &QMqttClient::stateChanged, this, &CommAcsSocketThread::cb_mqtt_state_changed);
    connect(m_client, &QMqttClient::messageReceived, this, &CommAcsSocketThread::cb_mqtt_message_recv);
    connect(m_client, &QMqttClient::pingResponseReceived, this, &CommAcsSocketThread::cb_mqtt_ping_recv);
    connect(m_client, &QMqttClient::errorChanged, this, &CommAcsSocketThread::cb_mqtt_error);

    m_client->setHostname(QString(MQTT_ADDRESS));
    m_client->setPort(MQTT_PORT);
    m_client->setAutoKeepAlive(true);
    //m_client->setUsername("sany");
    //m_client->setPassword("Sany@316");
    //m_client->setClientId("HMI");

    m_client->setWillTopic(MQTT_WILL_TOPIC);
    m_client->setWillQoS(1);
    QJsonObject json1;
    json1["online"] = false;
    QJsonDocument jsonDoc2(json1);

    QString willMessage(jsonDoc2.toJson());
    m_client->setWillMessage(willMessage.toLatin1());
    m_client->setWillRetain(true);

    m_client->connectToHost();

    connect(&m_time_handle, &QTimer::timeout, this, &CommAcsSocketThread::cb_on_time);
    m_time_handle.start(1000);
}

void CommAcsSocketThread::cb_on_time()
{
    if(m_client->state() == QMqttClient::Disconnected){
        TLOG_INFO("cb_on_time reconnect ");
        m_client->connectToHost();
    }
}

void CommAcsSocketThread::closeAllfortimer_up()
{
     TLOG_INFO("closeAllfortimer_up --lock on---end");
     closeAllwindow();
     TLOG_INFO("closeAllfortimer_up --lock off---start");
}

void CommAcsSocketThread::closeAllandShowUpdate(int index)
{
    TLOG_INFO("closeAllandShowUpdate");
    closeAllwindow();
    emit thread_to_update(index);
}

void CommAcsSocketThread::closeAllandforNormal()
{
    TLOG_INFO(" closeAllandforNormal ");
    closeAllwindow();
    this->cmdManager->lock_ui_off();
}

void CommAcsSocketThread::closeAllwindow()
{
    TLOG_INFO("close all the window ");
    this->widget_stack->push(this->cmdManager->window);
    int num = this->widget_stack->size();
    for(int i = 0;i < num - 1; i++)
    {
        this->cmdManager->window = widget_stack->pop();
        if(this->cmdManager->window == nullptr)
            continue;
        if(this->m_main_page == this->cmdManager->window)
            continue;
        delete this->cmdManager->window;
        this->cmdManager->window = nullptr;

    }
    this->cmdManager->window = this->widget_stack->pop();

    usleep(200000);
    QJsonObject jsonObj;
    jsonObj["main_page"] = 1;
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
    this->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toUtf8().data());
    if(this->control_timer != nullptr)
    {
        this->control_timer->stop();
    }
    update_flag_workface = 0;
    this->cmdManager->is_show_auxiliary = false;
}

CommAcsSocketThread::~CommAcsSocketThread()
{
    delete this->cmdManager;
    delete this->control_timer;
}

static void *pthread_handle(void *param)
{
    pthread_detach(pthread_self());
    CommAcsSocketThread *socked = static_cast<CommAcsSocketThread *>(param);
    prctl(PR_SET_NAME, "HMI_msg_pthread_handle");
    TLOG_WARN("pthread_handle is creating");
    while(true)
    {
        socked->cmdManager->m_semaphore->acquire();
        socked->last_run_time = time(nullptr);
        socked->cmdManager->m_mutex->lock();
        QString cmdline = socked->cmdManager->m_cmd_buf.front();
        socked->cmdManager->m_cmd_buf.pop_front();
        socked->cmdManager->m_mutex->unlock();
        TLOG_INFO("will handle msg is: %s", cmdline.toUtf8().data());
        socked->cmdManager->split_command(cmdline);
        socked->cmdManager->lock_ui_on();
        socked->cmdManager->parse_command();
    }
}

void CommAcsSocketThread::run(){

    pthread_t pid_handle;
    pthread_create(&pid_handle, nullptr, pthread_handle, this);
    int check_counts = 0;
    short version_send_times = 0;
    while(true)
    {
        sleep(1);
        if(++version_send_times == 1){
            QString msg = QString("config -hv ");

            msg += HMI_VERSION;
            send_socket_msg_to_acs(msg);
        }

        if(time(nullptr) - last_run_time < 3)
        {
            check_counts = 0;
            continue;
        }
        if( cmdManager->m_semaphore->available() <= 0 )
        {
            check_counts = 0;
            continue;
        }
        if(++check_counts < 2)
            continue;
        QDateTime date_time = QDateTime::fromTime_t(static_cast<unsigned int>(last_run_time));
        TLOG_ERROR("pthread_handle no work(last_handle_time:%d-%d-%d %d:%d:%d)", date_time.date().year(), date_time.date().month(),
                   date_time.date().day(), date_time.time().hour(), date_time.time().minute(), date_time.time().second());
        cmdManager->m_mutex->tryLock();
        cmdManager->m_mutex->unlock();
        cmdManager->lock_ui_off();
        last_run_time = time(nullptr);
    }
}

void CommAcsSocketThread::send_socket_msg_to_acs(const QString& msg)
{
    TLOG_INFO("send socket msg: %s", msg.toUtf8().data());
    return ;
}

void CommAcsSocketThread::send_socket_msg_to_acs(const QString &topic, const QByteArray& msg)
{
    TLOG_INFO("send socket msg: %s", msg.data());
    QMqttTopicName _topic(topic);
    m_client->publish(_topic, msg);
    return ;
}
