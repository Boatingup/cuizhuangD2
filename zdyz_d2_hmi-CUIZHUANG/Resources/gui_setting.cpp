#include "gui_setting.h"
#include "guiconfig.h"
#include <QDebug>
#include <QFile>
#include "error_code.h"
#include <QCoreApplication>
#include "./Resources/dbg_print.h"

extern Gui_config guisetting;
extern QList <mainPage_uigonfig> menuconfig_list;

Gui_config guisetting;
QList <mainPage_uigonfig> menuconfig_list;

gui_setting* gui_setting::mysettings = nullptr;

gui_setting::gui_setting()
{
    init();
}

void gui_setting::init()
{
    QString str_path = "/userdata/config/hmi_files";
    TLOG_INFO("str_path: %s\n", str_path.toUtf8().data());
    str_path += "/gui_setting.ini";

    QFile file(str_path);
    if(!file.exists())
    {
        TLOG_ERROR("gui setting not found");
    }

    settings_app = new QSettings(str_path, QSettings::IniFormat);
    settings_app->setIniCodec("utf-8");

}

void gui_setting::write(QString first,QString second,QString value)
{
    settings_app->beginGroup(first);
    settings_app->setValue(second,value);
    settings_app->sync();
    delete settings_app;
    init();
}

void gui_setting::getConfig()//获取方法二
{
    guisetting.sys_cfg.runOpcClient = settings_app->value("/systemConfig/runOpcClient").toBool();
    guisetting.sys_cfg.runOpcServer = settings_app->value("/systemConfig/runOpcServer").toBool();
    guisetting.sys_cfg.tcpServerIP= settings_app->value("/systemConfig/tcpServerIP").toString();
    guisetting.sys_cfg.tcpServerPort= settings_app->value("/systemConfig/tcpServerPort").toInt();
    guisetting.sys_cfg.hmiVersion = settings_app->value("/systemConfig/hmiVersion").toString();

    guisetting.sys_cfg.runTcpServer = settings_app->value("/systemConfig/runTcpServer").toBool();
    guisetting.sys_cfg.runTcpClient = settings_app->value("/systemConfig/runTcpClient").toBool();
    guisetting.sys_cfg.mutex_flag = settings_app->value("/test/mutex_ui").toBool();

    guisetting.update_cfg.pc_update_ip = settings_app->value("/UpdateConfig/pc_update_ip").toString();
    guisetting.update_cfg.ftp_user_Passwd = settings_app->value("/UpdateConfig/ftp_user_Passwd").toString();
    guisetting.update_cfg.ftp_user = settings_app->value("/UpdateConfig/ftp_user").toString();
    guisetting.update_cfg.ubang_ip = settings_app->value("/UpdateConfig/ubang_ip").toString();
    guisetting.update_cfg.file_path = settings_app->value("/UpdateConfig/file_path").toString();

    guisetting.access_cfg.is_access = settings_app->value("/AccessConfig/access_flag").toBool();
    guisetting.user_passwd_en = settings_app->value("/user_passwd/user").toString();
    guisetting.admin_passwd_en = settings_app->value("/user_passwd/admin").toString();
    guisetting.dev_passwd_en = settings_app->value("/user_passwd/dev").toString();

    int sync_num = settings_app->value("/sync_config/file_num").toInt();
    if(sync_num <= 0)
        return;
    int i, j;
    for(i=0; i<sync_num; i++)
    {
        QString file_config = QString("/sync_config/file_name_") + QString::number(i);
        QString file_name = settings_app->value(file_config).toString();
        guisetting.m_sync_file_name.append(file_name);

        QStringList path_list = file_name.split('/');
        QString path = QString(".");
        for(j=0; j<path_list.size()-1; j++)
            path += path_list.at(j) + QString("/");
        guisetting.m_sync_file_path.append(path);
    }
}

