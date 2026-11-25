#include <QApplication>
#include <QWidget>
#include <QThread>
#include <QCursor>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextCodec>

#include "./window/mainpage.h"
#include "./window/menupage.h"
#include "./window/leftpane.h"
#include "./threads/comm_acs_thread.h"
#include "./threads/para_synchronize_thread.h"
//#include "./threads/para_synchronize_file.h"
#include "./threads/update_thread.h"
#include "./threads/comm_acs_socket_thread.h"
#include "./threads/can_param_sync.h"

#include <stdio.h>
#include <signal.h>
#include "show_version.h"
//for test ui css ----start
#include "dialog_usercheckandlogin.h"
#include "show_runerror.h"
#include "./Resources/mqtt_message.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
#include "Resources/dbg_print.h"
//for test ui css ----end

extern QList <mainPage_uigonfig> menuconfig_list;

// 定义一个全局的zlog日志上下文
zlog_category_t *g_zlog_ctx;

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    QApplication a(argc, argv);
    setbuf(stdout, nullptr);

    QWidget * pMainPage = new mainPage();
    pMainPage->show();

    /*socket message communication*/
    CommAcsSocketThread* comm_acs_socket_thread = CommAcsSocketThread::getInstance();

    printf("-------------------------------\n");
    comm_acs_socket_thread->cmdManager->window = pMainPage;
    comm_acs_socket_thread->m_main_page = pMainPage;
    comm_acs_socket_thread->start();

    CanParamSync::instance()->init();

    return a.exec();
}
