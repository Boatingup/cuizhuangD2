#ifndef COMM_ACS_THREAD_H
#define COMM_ACS_THREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "../Resources/command.h"
#include "widget_stack.h"

#define KEY_PATH                    "/usr/bin/"
#define KEY_ID_FROM_ACS_TO_HMI		'a'
#define KEY_ID_FROM_HMI_TO_ACS		'h'
#define MSG_TYPE                    100
#define BUF_SIZE_MSG                    128

#define MSG_TYPE_TO_UPDATE_PROGRAM  101


typedef struct {
    long mtype;
    char mtext[BUF_SIZE_MSG];
}msgbuf_t;

class CommAcsThread : public QThread
{
    Q_OBJECT
private:
    CommAcsThread();
public:
    ~CommAcsThread();

public:
    static CommAcsThread* getInstance(){
        if(com_acs_thread == nullptr){
            com_acs_thread = new CommAcsThread();
        }
        return com_acs_thread;
    }

protected:
    void run();
public slots:
    void closeAllwindow();
    void closeAllandShowUpdate(int index);
    void closeAllandforNormal();
    void closeAllfortimer_up();


public:
    void send_msg_to_acs(const QString& msg, int msg_type = MSG_TYPE);
    bool cmdCheck(char *buf);
    WidgetStack * widget_stack;

private:
    int key_snd;
    int key_rcv;
    int msgid_snd;
    int msgid_rcv;

private:
    msgbuf_t msg_buf;

private:
    static CommAcsThread *com_acs_thread;
public:
    CmdManager* cmdManager;
    QTimer * control_timer;
public:
    int main_control_time;
signals:
    void thread_to_update(int index);
private:
//    QWidget* window; //current window
};

#endif // COMM_ACS_THREAD_H
