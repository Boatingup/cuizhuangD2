#ifndef UPDATE_THREAD_H
#define UPDATE_THREAD_H

#include <QObject>
#include <QThread>
#include "../Resources/parameters.h"
#include "../threads/comm_acs_socket_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

class UpdateThread : public QThread
{
    Q_OBJECT
private:
    UpdateThread();
public:
    ~UpdateThread(){}

    static UpdateThread* getInstance(){
        if(pup == nullptr){
            pup = new UpdateThread();
        }
        return pup;
    }

    static UpdateThread* pup;
    ActionRegister* action_register;

protected:
    void run();

private:
    Parameters* para;
    WidgetStack* widget_stack;
    CommAcsSocketThread* comm_acs_thread;

signals:
    void from_thread_update(int index);

};

#endif // PARA_SYNCHRONIZE_THREAD_H
