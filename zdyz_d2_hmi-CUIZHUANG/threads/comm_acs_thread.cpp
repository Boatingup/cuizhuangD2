#include "comm_acs_thread.h"
#include "comm_acs_socket_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>

#include <string>

#include "../Resources/command.h"
#include <QDebug>
#include <QByteArray>
#include "../Resources/error_code.h"

CommAcsThread* CommAcsThread::com_acs_thread = nullptr;

extern int update_flag_workface;

CommAcsThread::CommAcsThread()
{
    key_rcv = ftok(KEY_PATH, KEY_ID_FROM_ACS_TO_HMI);
    key_snd = ftok(KEY_PATH, KEY_ID_FROM_HMI_TO_ACS);
    if(key_snd < 0 || key_rcv < 0){
        LOG(INFO) << "create key failed.";
    }

    msgid_snd = msgget(key_snd, IPC_CREAT | 0666);
    msgid_rcv = msgget(key_rcv, IPC_CREAT | 0666);
    if(msgid_rcv < 0 || msgid_snd < 0){
        LOG(INFO) << "create msg id failed.";
    }

    this->cmdManager = new CmdManager();
}

CommAcsThread::~CommAcsThread(){
    if(msgctl(msgid_rcv, IPC_RMID, nullptr) < 0 || msgctl(msgid_snd, IPC_RMID, nullptr) < 0){
        LOG(INFO) << "delete message queue failed.";
    }
    delete this->cmdManager;
    //delete this->control_timer;
}
bool CommAcsThread::cmdCheck(char *buf)
{
    string str(buf);
    if(str.find("lock") == string::npos && str.find("stop") == string::npos)
    {
        return false;
    }
    return true;
}
void CommAcsThread::run(){
    while(true){
        if(msgrcv(msgid_rcv, &msg_buf, sizeof(msgbuf_t)-sizeof(long), MSG_TYPE, 0) >= 0){
            DBG_INFO("hmi msg_queue recieved:%s\n", msg_buf.mtext);
            LOG(INFO) << "hmi msg_queue recieved:" << msg_buf.mtext;
            if(strlen(msg_buf.mtext) <= 1 || !cmdCheck(msg_buf.mtext))
            {
                continue;
            }
#if 0
            cmdManager->m_mutex->lock();
            cmdManager->m_cmd_buf.push_back(msg_buf.mtext);
            cmdManager->m_mutex->unlock();
            cmdManager->m_semaphore->release();
#else
            QString cmdline(msg_buf.mtext);
            cmdManager->split_command(cmdline);
            cmdManager->parse_command();
#endif
        }
    }
}

