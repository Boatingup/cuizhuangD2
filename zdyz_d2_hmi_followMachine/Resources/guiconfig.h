#ifndef GUICONFIG_H
#define GUICONFIG_H

struct SystemConfig{
    QString hmiVersionMax;//version_max
    QString hmiVersionMin;//version_min
    QString dataTransferType;//tcp or msgqueue
    QString MsqQueueRecvtype;//block or not block
    int tcpclientContime;//client connect_time
    QString tcpServerIP;//tcp server ip
    int tcpServerPort;//tcp server port
    bool runOpcClient = 0;//start opcClient flag
    bool runOpcServer = 0;//start opcServer flag
    QString hmiVersion; //hmi version
    bool runTcpServer = 0;
    bool runTcpClient = 0;
    bool mutex_flag = 1;

};

struct UpdateConfig{

    QString ftp_user;
    QString ftp_user_Passwd;
    QString pc_update_ip;
    QString ubang_ip;  
    QString file_path;
};

struct AccessConfig{
    bool is_access = 0;//是否打开了权限管理
};

struct mainPage_uigonfig{
    QString page;//当前页面名称
    QString page_chn;//当前页面中文名
    int access;//可进入
    int write_available;//写
    bool haschild;//是否有子窗口
    int class_level;//是几级窗口
    QList <mainPage_uigonfig> nextclass_page;//下一级菜单
};

struct healthy_check_uiconfig{
    int test = 1;

};

struct Gui_config{
    //UI_config ui_cfg;
    SystemConfig sys_cfg;
    UpdateConfig update_cfg;
    AccessConfig access_cfg;
    QString admin_passwd;
    QString user_passwd;
    QString dev_passwd;
    QString goal_position;

    QString user_passwd_en;
    QString admin_passwd_en;
    QString dev_passwd_en;

    QList<QString> m_sync_file_name;
    QList<QString> m_sync_file_path;
};

#endif // GUICONFIG_H
