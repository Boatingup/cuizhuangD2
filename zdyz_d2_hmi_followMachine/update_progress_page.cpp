#include "update_progress_page.h"
#include "ui_update_progress_page.h"

#include "Resources/key_value.h"
#include "threads/comm_acs_thread.h"

#include <QEvent>
#include <QKeyEvent>
#include "./Resources/error_code.h"

#include <QString>
#include "./Resources/tcpclient.h"
#include "./Resources/gui_setting.h"
#include <QProcess>
#include <QThread>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QDebug>
#include <QProgressBar>
//#include "host.h"
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include "syncparafile.h"

extern Gui_config guisetting;
extern bool serverflag;
extern struct Update_info update_info;
extern int update_flag_workface;

extern int percent;
extern bool updating;//本机操作标志位
extern bool pc_update_flag;//上位机升级标志位
extern bool cancel_flag_process;
extern bool g_support_update_flag;

int percent = 0;
bool updating = false;//本机操作标志位
bool pc_update_flag = 0;//上位机升级标志位
bool cancel_flag_process = 0;

extern TcpClient *tcp_client;

void UpdateProgressPage::Init()
{
    list_update << "ACS" << "HMI" << "gui_setting.ini" << "parameters.xml" << "depend_files/action.xml" << "ServerConfig.ini" << "SocketServer";
    up_finished =1;
    connect(this,SIGNAL(signal_files(QString)),this,SLOT(downloadfiles(QString)));
}

int UpdateProgressPage::VersionCheck()
{
    file = new QFile(QApplication::applicationDirPath() + QString("/") + QString("parameters_update.xml"));
    file->open(QIODevice::WriteOnly);

    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QString url_str;
    if((update_index == 2) || (update_index == 4))
    {
        url_str = "ftp://" + ip_server + QString(file_path) +  QString("parameters.xml");
    }
    else if((update_index == 1) || (update_index == 3))
    {
        url_str = "ftp://" + ip_server + QString(file_path) +  QString("parameters.xml");
    }

    QUrl url(url_str);
    url.setPort(21);
    url.setUserName(user);
    url.setPassword(passwd);

    QNetworkRequest request(url);
    reply = accessManager->get(request);

    LOG(INFO) << " start update " << url_str.toUtf8().data();
    LOG(INFO) << " user name: " << user.toUtf8().data() << " ; password: " << passwd.toUtf8().data();
    ui->progressBar->setValue(0);
    ui->progressBar->show();

    connect(static_cast<QObject *>(reply), SIGNAL(readyRead()), this, SLOT(readContent()));
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(CheckreplyFinished(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(CheckloadError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(CheckloadProgress(qint64 ,qint64)));
    return 0;
}

int UpdateProgressPage::executeCMD(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=nullptr)
    {
        while(fgets(buf_ps, 1024, ptr)!=nullptr)
        {
           strcat(result, buf_ps);
           if(strlen(result)>1024)
               break;
        }
        pclose(ptr);
        ptr = nullptr;
        return 0;
    }
    else
    {
        printf("popen %s error\n", ps);
        return 1;
    }
}

int UpdateProgressPage::sleepReboot()
{
    sleep(10);
    system("/sbin/reboot");
    LOG(INFO) << "sleepReboot system reboot";
    return 0;
}

int UpdateProgressPage::BeforeReboot()
{
    if(Curitem == "all")
    {
        unlink("/home/root/HMI");
        unlink("/home/root/ACS");
        unlink("/home/root/SocketServer");
        unlink("/home/root/listenserver.sh");
        rename("/home/root/HMIBAK", "/home/root/HMI");
        rename("/home/root/ACSBAK", "/home/root/ACS");
        rename("/home/root/SocketServerBAK", "/home/root/SocketServer");
        rename("/home/root/listenserver.shBAK", "/home/root/listenserver.sh");
        chmod("/home/root/SocketServer", 0777);
        chmod("/home/root/listenserver.sh", 0777);
    }

    LOG(INFO) << "before reboot";

    QEventLoop eventloop;
    QTimer::singleShot(10000,&eventloop,SLOT(quit()));
    eventloop.exec();
    sync();
    LOG(INFO) << "BeforeReboot reboot ";
    reboot(RB_AUTOBOOT);
    return 0;
}

void UpdateProgressPage::reboot_exec()
{
    system("/sbin/reboot");
    LOG(INFO) << "reboot_exec system reboot";
}

int UpdateProgressPage::renewacsVersion(QString name)
{
    //QString value_hmi(argv[2]);
    int flag_hmi = -1;
    QString category_name = "set_para";
    QString tab_name = "controller_info";
    QString parameter_name = "version_record";
    QString label = "acs_version";
    LOG(INFO) << "judge parameters.xml-----4";
    flag_hmi = para->update(QString("parameters.xml"),category_name,tab_name,parameter_name,label,name);
    if(flag_hmi == 0)
    {
        DBG_INFO_OLD("Success!\n");
        return 0;
    }
    else
    {
        DBG_INFO_OLD("Failed!\n");
        return 1;
    }
}

int UpdateProgressPage::renewhmiVersion(QString name)
{
    //QString value_hmi(argv[2]);
    int flag_hmi = -1;
    QString category_name = "set_para";
    QString tab_name = "controller_info";
    QString parameter_name = "version_record";
    QString label = "hmi_version";
    LOG(INFO) << "judge parameters.xml --------5";
    flag_hmi = para->update(QString("parameters.xml"),category_name,tab_name,parameter_name,label,name);
    if(flag_hmi == 0)
    {
        DBG_INFO_OLD("Success!\n");
        return 0;
    }
    else
    {
        DBG_INFO_OLD("Failed!\n");
        return 1;
    }
}

QByteArray UpdateProgressPage::get_file_hash(QString &file_name)
{
    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();
    return hash;
}

void UpdateProgressPage::CheckreplyFinished(QNetworkReply*)
{
    LOG(INFO) << "check -----------0 " << reply->error() ;
    if(reply->error() == QNetworkReply::NoError)
    {
        reply->deleteLater();
        file->flush();
        file->close();
    }
    else
    {
        LOG(INFO) << "check -----------1122 " << reply->error();
        reply->deleteLater();
        ui->label_process->setText("版本校验： 参数文件下载失败");
        return;
    }
    char result[2048] = {0};
    char acs_version[64] = "/home/root/VersionTools -ra";
    char hmi_version[64] = "/home/root/VersionTools -rh";
    LOG(INFO) << "here -------failed  " << Curitem.toStdString().c_str();
    const char *p = static_cast<const char *>(Curitem.toStdString().c_str());
    LOG(INFO) << "length: " << strlen(p);
    LOG(INFO) << "Curitem: " <<  Curitem.toStdString().c_str();
    if(Curitem == "ACS")
    {
        int rtn = executeCMD(acs_version, result);
        {
               if(rtn == 0)
               {
                   QString content(result);
                   QStringList list = content.split(":");
                   if(list[0] == "acs")
                   {
                       acs = list[1];
                       acs = acs.left(acs.size() - 1);
                        downloadButton_clicked();
                   }
               }
               else
               {
                   ui->label_process->setText("ACS 版本获取失败");
                   return;
               }
        }
    }
    else if(Curitem == "HMI")
    {
        int rtn = executeCMD(hmi_version, result);
        {
               if(rtn == 0)
               {
                   QString content(result);
                   QStringList list = content.split(":");
                   if(list[0] == "hmi")
                   {
                       hmi = list[1];
                       //add this version not equal
                       hmi = hmi.left(hmi.size() - 1);
                        downloadButton_clicked();

                   }
                   else
                   {
                       ui->label_process->setText("HMI 版本解析失败");
                       return;
                   }
               }
               else
               {
                   ui->label_process->setText("HMI 版本获取失败");
                   return;
               }
        }
    }
    else if(Curitem == "all")
    {
        downloadButton_clicked();
    }
    else
    {
        downloadButton_clicked();
    }

}

void UpdateProgressPage::CheckloadError(QNetworkReply::NetworkError)
{
     DBG_INFO_OLD("Error: %d\n", reply->error());

}

void UpdateProgressPage::downloadButton_clicked()
{
    if(Curitem != "all")
    {
        file = new QFile(QApplication::applicationDirPath() + QString("/") + Curitem + QString("BAK"));
        file->open(QIODevice::WriteOnly);

        accessManager = new QNetworkAccessManager(this);
        accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);

        QString url_str;

        if((update_index == 2) || (update_index == 4))
        {
           url_str = "ftp://" + ip_server + QString(file_path) +  Curitem;
        }
        else if((update_index == 1) || (update_index == 3))
        {
           url_str = "ftp://" + ip_server + QString(file_path) +  Curitem;
        }
        LOG(WARNING) << url_str.toUtf8().data();
        QUrl url(url_str);

        url.setPort(21);
        url.setUserName(user);
        url.setPassword(passwd);

        QNetworkRequest request(url);
        reply = accessManager->get(request);

        ui->progressBar->setValue(0);
        ui->progressBar->show();

        connect(static_cast<QObject *>(reply), SIGNAL(readyRead()), this, SLOT(readContent()));
        connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(loadProgress(qint64 ,qint64)));

    }
    else
    {
        if(list_update.size() > index)
        {
            emit signal_files(list_update[index]);
        }
    }

}

void UpdateProgressPage::readContent()
{
    LOG(INFO) << "downloadsuccessfully";
    file->write(reply->readAll());
    LOG(INFO) << "readContent()";
}


void UpdateProgressPage::downloadfiles(QString file_fp)
{
    if(ui->label_title->text().contains("all") == true)
        ui->label_title->setText("all:" + list_update.at(this->index));
    //LOG(INFO) << "item : " << file_fp.toStdString().c_str();

    file = new QFile(QApplication::applicationDirPath() + QString("/") + QString(file_fp) + QString("BAK"));
    file->open(QIODevice::WriteOnly);

    accessManager = new QNetworkAccessManager(this);
    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QString url_str;

    if((update_index == 2) || (update_index == 4))
    {
       url_str = "ftp://" + ip_server + QString(file_path) +  QString(file_fp);
    }
    else if((update_index == 1) || (update_index == 3))
    {
       url_str = "ftp://" + ip_server + QString("/") +  QString(file_fp);
    }
    QUrl url(url_str);
    url.setPort(21);
    url.setUserName(user);
    url.setPassword(passwd);
    QNetworkRequest request(url);
    reply = accessManager->get(request);

    ui->progressBar->setValue(0);
    ui->progressBar->show();
    connect(static_cast<QObject *>(reply), SIGNAL(readyRead()), this, SLOT(readContent()));
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(loadProgress(qint64 ,qint64)));
    index++;
}

void UpdateProgressPage::replyFinished(QNetworkReply*)
{
    LOG(INFO) << "in replyFinished";
    if(Curitem != "all")
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            reply->deleteLater();
            file->flush();
            file->close();
            QString file_name = "/home/root/" + Curitem;
            QString back_file_name = "/home/root/" + Curitem + "BAK";

            if(get_file_hash(file_name).contains(get_file_hash(back_file_name)))
            {
                //unlink(back_file_name.toUtf8().data());
                if(g_support_update_flag)
                    g_support_update_flag = false;
                ui->label_process->setText("版本一致,无需升级,按取消键退出!");
                if(strcmp(update_info.status.c_str(), "4") == 0)
                    update_info.status = "0";
                update_flag_workface = 0;
                if(!updating)
                    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
                return ;
            }
            unlink(file_name.toUtf8().data());
            rename(back_file_name.toUtf8().data(), file_name.toUtf8().data());
            chmod(file_name.toUtf8().data(), 0777);

            if(Curitem == "rootfs.tar.gz"){
                ui->label_process->setText("文件下载成功,等待升级,预计3分钟！");
                LOG(INFO) << "download rootfs.tar.gz";
                system("tar -xzvf rootfs.tar.gz -C / ");
            }
            ui->label_process->setText("升级成功! 10 秒后重启！");
            BeforeReboot();
        } else {
            LOG(ERROR) << reply->error();
            reply->deleteLater();
            ui->label_process->setText("升级失败，按确认键重新升级，按取消键退出");
            return;
        }
    } else {
        if(reply->error() == QNetworkReply::NoError)
        {
            file->flush();
            file->close();    
            LOG(INFO) << "download a file";
            if(Curitem == "all")
            {
                if(index < list_update.size())
                {
                    emit signal_files(list_update[index]);
                    ui->progressBar->setValue(0);
                }
                else
                {
                    ui->label_process->setText("升级成功! 10 秒后重启！");
                    BeforeReboot();
                }
            }
        } else {
            LOG(ERROR) << reply->error();
            up_failed = 1;
            ui->label_process->setText("升级失败,网络错误");
            return;
        }
    }
}

void UpdateProgressPage::CheckloadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if(bytesSent == bytesTotal){
    }
}

void UpdateProgressPage::loadProgress(qint64 bytesSent, qint64 bytesTotal)  {
    ui->progressBar->setMaximum(static_cast<int>(bytesTotal)); //最大值
    ui->progressBar->setValue(static_cast<int>(bytesSent));  //当前值
    if(Curitem == "all")
    {
        percent = static_cast<int>((index / list_update.size()) * 100);//(float )(index / list_update.size()) * 100;
    }
    else
    {
        if(bytesTotal > 0)
        {
            percent = static_cast<int>((bytesSent / bytesTotal ) * 100);//(float) (bytesSent / bytesTotal ) * 100;
        }
    }
}

void UpdateProgressPage::loadError(QNetworkReply::NetworkError)
{
     LOG(ERROR) << "update failed : " << reply->error();
}

void UpdateProgressPage::UpdateProcessUI(int num,int index,int process,int status,QString content)
{
    if((num == index)||(process == status)||(content.isNull())){
    }
    Init();
    VersionCheck();
}

int UpdateProgressPage::GetStylePath(QString &css)
{
    QFile file(":/qss/update_progress_page.qss");
    int m = file.open(QFile::ReadOnly);
    if (m == true)
    {
        css = file.readAll(); //获取qss中全部字符
        return 0;
    }
    else
    {
        return 1;
    }
}

QString UpdateProgressPage::getCurNum()
{
    QString category_name("system");
    QString tab_name("system_para");
    QString parameter_name("updateCurnum");
    QString label("real_value");
    //QString test = this->para->query(category_name,tab_name,parameter_name,label);
    return(this->para->query(category_name,tab_name,parameter_name,label));

}

UpdateProgressPage::UpdateProgressPage(QString title,int index,QString item,QWidget *parent) :
    QWidget(parent),Curitem(item),ui(new Ui::UpdateProgressPage),update_title(title),update_index(index)
{
    ui->setupUi(this);

    ui->label_title->setText(title);

    LOG(INFO) << "update info "  << title.toStdString().c_str() << " "  << index << " " << item.toStdString().c_str();
    //InitEnv();
    QString css;
    int rtn = GetStylePath(css);
    if(rtn == 0)
    {
        this->setStyleSheet(css); //设置样式表
    }
    else
    {
        LOG(INFO) << "load css failed";
    }

    comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();

    user = guisetting.update_cfg.ftp_user;
    passwd = guisetting.update_cfg.ftp_user_Passwd;
    file_path = guisetting.update_cfg.file_path;

    if((update_index == 1) || (update_index == 3))
    {
        ip_server = guisetting.update_cfg.pc_update_ip;
    }
    if((update_index == 2) || (update_index == 4))
    {
        ip_server = guisetting.update_cfg.ubang_ip;
    }

    accessManager = new QNetworkAccessManager(this);
    ui->progressBar->setValue(0);
    Init();
    VersionCheck();
    ui->label_process->setText("升级中.....成功后将重启!");

}

QString UpdateProgressPage::getAcsVersion()
{
    QString category_name("set_para");
    QString parameter_name("version_record");
    QString label("acs_version");
    return(this->para->query(category_name,0,parameter_name,label));
}

QString UpdateProgressPage::getHmiVersion()
{
    QString category_name("set_para");
    QString parameter_name("version_record");
    QString label("hmi_version");
    QString version = (this->para->query(category_name,0,parameter_name,label));
    return version;
}

UpdateProgressPage::~UpdateProgressPage()
{
    delete ui;
    if(accessManager != nullptr)
    {
        delete accessManager;
        accessManager = nullptr;
    }
}

bool UpdateProgressPage::event(QEvent *event)
{
    //char buf[128] = {0};
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_OK:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            update_info.status = "0";
            if((update_index == 1) || (update_index == 2))
            {
                update_flag_workface = 0;
            }
            else if(update_index == 3)
            {
                update_flag_workface = 0;
            }
            else if(update_index == 4)
            {
                update_flag_workface = 0;
            }

            if(pc_update_flag == 1)
            {
                pc_update_flag = 0;
            }
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~UpdateProgressPage();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
