#ifndef PARA_SYNCHRONIZE_THREAD_H
#define PARA_SYNCHRONIZE_THREAD_H

#include <QObject>
#include <QThread>
#include "../Resources/parameters.h"
#include "../Resources/gui_setting.h"
#include "comm_acs_socket_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

class ParaSynchronizeThread : public QObject
{
    Q_OBJECT
private:
    ParaSynchronizeThread(QObject *parent = nullptr);
public:
    ~ParaSynchronizeThread(){}


public:
    static ParaSynchronizeThread* getInstance(){
        if(pst == nullptr){
            pst = new ParaSynchronizeThread();
        }
        return pst;
    }

public:
    void send_para_synchronization_msg(const QString& msg);
    void parse_msg(const QByteArray &message);
    void parse_msg1(const QByteArray &message);

signals:
    void sig_update_slogan(const QString &slogan);

public:
    static ParaSynchronizeThread* pst;
    ActionRegister* action_register;

private:
    Parameters* para;
    //int send_sock;
    //struct sockaddr_in broad_adr;
    WidgetStack* widget_stack;
    gui_setting *gui_set;

private:
    CommAcsSocketThread* comm_acs_thread;
};

#endif // PARA_SYNCHRONIZE_THREAD_H
