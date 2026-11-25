#ifndef COMM_ACS_SOCKET_THREAD_H
#define COMM_ACS_SOCKET_THREAD_H

#include <QObject>
#include <QThread>
#include <QMqttClient>

#include "../Resources/command.h"
#include "widget_stack.h"

#define SOCK_PORT           5050
#define SOCK_IPADDR_LENGTH  16
#define SOCK_BUF_SIZE       1024

typedef struct {
    char ipaddr[SOCK_IPADDR_LENGTH];
    short port;
    char buf[SOCK_BUF_SIZE];
    void *reserve;
}socketmsgbuf_t;

class CommAcsSocketThread : public QThread
{
    Q_OBJECT
private:
    CommAcsSocketThread();
public:
    ~CommAcsSocketThread();

public:
    static CommAcsSocketThread* getInstance(){
        if(com_acs_socket_thread == nullptr){
            com_acs_socket_thread = new CommAcsSocketThread();
        }
        return com_acs_socket_thread;
    }

protected:
    void run();
public slots:
    void closeAllwindow();
    void closeAllandShowUpdate(int index);
    void closeAllandforNormal();
    void closeAllfortimer_up();

    void cb_mqtt_connected();
    void cb_mqtt_disconnected();
    void cb_mqtt_state_changed(QMqttClient::ClientState state);
    void cb_mqtt_message_recv(const QByteArray &message, const QMqttTopicName &topic = QMqttTopicName());
    void cb_mqtt_ping_recv();
    void cb_mqtt_error(QMqttClient::ClientError error);

    void cb_send_hmi_version(const QByteArray msg);
    void cb_on_time();

public:
    int create_tcp_connection();
    void send_socket_msg_to_acs(const QString& msg);
    void send_socket_msg_to_acs(const QString &topic, const QByteArray& msg);
    void make_tcp_connected();
    int get_connect_state(int *state);
    int get_local_ipaddr(char *ip_buf);
    bool cmdKeyCheck(char *buf);
    void add_socked_list(int socked);
    void remove_socked_list(int socked);

    void mqtt_init();

private:
    socketmsgbuf_t msg_buf;
    QTimer m_time_handle;
    static CommAcsSocketThread *com_acs_socket_thread;
public:
    QWidget *m_main_page;
    CmdManager* cmdManager;
    QTimer * control_timer;
    QList<int> m_socked;
    int main_control_time;
    int m_alter_control_time;
    int m_group_control_time;
    time_t last_run_time;
    pthread_mutex_t m_socked_mutex;
    WidgetStack * widget_stack;
    /*adapt mqtt*/
    QMqttClient *m_client;
signals:
    void thread_to_update(int index);
    void inform_fault_status(int level);
    void inform_net_status(int level);

};

#endif // COMM_ACS_SOCKET_THREAD_H
