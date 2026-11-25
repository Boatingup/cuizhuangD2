#ifndef UPDATE_PROGRESS_PAGE_H
#define UPDATE_PROGRESS_PAGE_H

#include <QWidget>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"
#include <QStandardItemModel>
#include "./Resources/parameters.h"
#include <QNetworkReply>
//#include <QThread>

namespace Ui {
class UpdateProgressPage;
}

class UpdateProgressPage : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateProgressPage(QString title,int index,QString Curitem,QWidget *parent = nullptr);
    ~UpdateProgressPage();
private:
    QString Curitem;
    QString hmi,acs;
    //int exestatus = 0;
    QString getHmiVersion();
    QString getAcsVersion();
    QByteArray get_file_hash(QString &file_name);
    int sleepReboot();
    int BeforeReboot();
    QString getCurNum();
    int executeCMD(const char *cmd, char *result);

    QString ip_server;
    QString user;
    QString passwd;
    QString file_path;

    //void DeleteAccess();

    //QString getCurnum();
    //int CreatDir(QString path);
    //int CheckDir(QString ip,QString path);
    //int CreatDir(QString ip,QString path);
    //int getIP(QString num,QString & ip);
    //int execcmd(QString cmd);
    //int change_link(QString ip);
    //int reboot(QString ip);
    //int scpfile_fromUbang(QString ubip,QString src_ubang_path,QString local_path);
    //int scppath_fromUbang(QString ubip,QString src_ubang_path,QString local_path);
    //int scpfile_toOther(QString otherip,QString local_path, QString dest_path);
    //int scppath_toOther(QString otherip,QString local_path, QString dest_path);
    //int UpdateProcessor(int num);
    //int UpdateProcessor();
    //int getIP(QString num,QString & ip);
    void OpenUpdateThread(QString item,int num_ip,int num,QString Currip);
    void CloseUpdateThread();//关闭升级线程
    int VersionCheck();

    /*
    QString UBang_Passwd;// "syzk";
    QString Process_Passwd;// "syzk";
    QString Ubang_IP;// "192.168.132.15";
    QString Process_IP_prefix;// "192.168.132.25";
    QString Bak_Path_Ubang;//  "/home/root/syzk_new";
    QString Bak_Path_Local;// "/home/root/syzk_new";
    QString Bak_Path_Others;//  "/home/root/syzk_new";
    QString Real_path;// = "/home/root/syzk";
    QString last_Path_Others;// = "/home/root/syzk_old";
    QString Work_path;

    QString Cmd_pass = "sshpass -p syzk ";
    QString Cmd_Copy_Scp_file = " scp ";
    QString Cmd_Copy_Scp_path = " scp -r ";
    QString Cmd_Copy_mkdir = " mkdir -p ";
    QString Cmd_Check_dir = " ls ";
    */
    QString Currip;
    QStringList list_update;
    int up_finished = 0;
    int up_failed = 0;
    QNetworkAccessManager *accessManager;
    /*
    int step_four();
    int step_three();
    int step_two();
    int step_one(int num);
    */
private slots:
    //void on_uploadButton_clicked();
    //void on_downloadButton_clicked();
    void readContent();
    void replyFinished(QNetworkReply*);
    void loadError(QNetworkReply::NetworkError);
    void loadProgress(qint64 bytesSent,qint64 bytesTotal);
    //void on_uploadButton_clicked();
    void downloadButton_clicked();
    void CheckreplyFinished(QNetworkReply*);
    void CheckloadError(QNetworkReply::NetworkError);
    void CheckloadProgress(qint64 bytesSent,qint64 bytesTotal);

    int renewacsVersion(QString name);
    int renewhmiVersion(QString name);


public slots:
    //void update_progress(int);
    //void stopUpdate();
    //void reUpdate();
    //void update_show_fail();
    //void InitEnv();
    //void readBashStandardOutputInfo();
    //void readBashStandardErrorInfo();
    //void UpdateUI(int,int,int,QString);
    void UpdateProcessUI(int num,int index,int process,int status,QString content);
    void Init();
    void downloadfiles(QString file);

    void reboot_exec();

protected:
    bool event(QEvent *event);

private:
    Ui::UpdateProgressPage *ui;
    int GetStylePath(QString &css);
private:
    CommAcsSocketThread* comm_acs_thread;
    QString update_title;
    int update_index;
    int update_flag = 1;
    int index = 0;
    Parameters* para;

    QProcess m_proces_bash;
    QThread *updatethread = nullptr;

    int status = -1;

private:
    WidgetStack* widget_stack;
    QFile *file;
    QNetworkReply *reply;

    QString acs_v;
    QString hmi_v;
signals:
    void operate(QString str);
    void signal_files(QString file);

};

#endif // UPDATE_PROGRESS_PAGE_H
