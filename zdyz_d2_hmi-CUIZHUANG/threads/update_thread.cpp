#include "update_thread.h"
#include "./Resources/error_code.h"
#include <mutex>
#include "update_progress_page.h"
//#include "host.h"
#include <QApplication>
#include "open62541.h"

extern struct Update_info update_info;
extern int update_flag_workface;
extern char ip_client[16];
extern bool updating;
extern bool pc_update_flag;
extern UA_Client *client;
extern void ReadSUpdateerverIP(char * sstatus,UA_Client *client);

UpdateThread* UpdateThread::pup = nullptr;

UpdateThread::UpdateThread()
{
    this->para = Parameters::getInstance();
    
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->action_register = ActionRegister::getInstance();
    this->widget_stack = WidgetStack::getInstance();
}


void UpdateThread::run()
{
    while(true)
    {
        //TLOG_INFO("--------------update_flag_workface:%d------update_info:%s----------\n", update_flag_workface, update_info.status.c_str());
        if(update_flag_workface == 1)//上位机工作面升级
        {
            emit from_thread_update(update_flag_workface);
            update_flag_workface = 5;
        }
        else if(update_flag_workface == 3)//上位机单台升级
        {
            emit from_thread_update(update_flag_workface);
            update_flag_workface = 5;
        }
        else if(update_flag_workface == 2)//控制器工作面升级
        {
            emit from_thread_update(update_flag_workface);
            update_flag_workface = 5;
        }
        else if (update_flag_workface == 4)//控制器单台升级
        {
            emit from_thread_update(update_flag_workface);
            update_flag_workface = 5;
        }
        else if (update_flag_workface == 6)//update error with no opc serevr
        {
            emit from_thread_update(update_flag_workface);
            update_flag_workface = 5;
        }
        usleep(200000);
    }
}


