#include "syncparafile.h"
#include "ui_syncparafile.h"
#include "config_backup_restore.h"
#include <QUdpSocket>
#include <QFile>
#include "./threads/comm_acs_thread.h"
#include <QtNetwork>
#include <QDebug>
#include <QList>
#include <QNetworkInterface>
#include <QEvent>
#include <QKeyEvent>
#include "./Resources/key_value.h"
#include <unistd.h>

SyncParaFile::SyncParaFile(FILE_TYPE_E type, QWidget *parent, const QString &file_path) :
    QWidget(parent),
    ui(new Ui::SyncParaFile),
    file_type(type),
    restore_file_path(file_path),
    operation_done(false)
{
    ui->setupUi(this);

    comm_acs_thread = CommAcsSocketThread::getInstance();
    widget_stack = WidgetStack::getInstance();
    para = Parameters::getInstance();

    if(type == TYPE_PARAM){
        ui->label->setText("工作面正在同步参数配置文件，请稍后...");
        ui->label_2->setText("同步完成后自动重启");
    } else if(type == TYPE_DRIVER){
        ui->label->setText("工作面正在同步驱动器配置文件，请稍后...");
        ui->label_2->setText("同步完成后自动重启");
    } else if(type == TYPE_DEFAULT_PARAM){
        ui->label->setText("正在恢复出厂设置，预计1分钟");
        ui->label_2->setText("恢复出厂设置完成后自动重启");
    } else if(type == TYPE_PARAM_DELAY){
        ui->label->setText("工作面正在同步参数配置文件，请稍后...");
        ui->label_2->setText("同步完成后自动重启");
    } else if(type == TYPE_VERSION){
        ui->label->setText("工作面正在同步版本，请稍后...");
        ui->label_2->setText("同步完成后自动重启");
    } else if(type == TYPE_CONFIG_BACKUP){
        ui->label->setText("正在备份配置文件，请稍后...");
        ui->label_2->setText("备份完成后按取消返回");
    } else if(type == TYPE_CONFIG_RESTORE){
        ui->label->setText("正在恢复配置文件，请稍后...");
        ui->label_2->setText("恢复完成后自动重启");
    }
    connect(this, SIGNAL(SyncPara()), this, SLOT(SyncParaFiles()));
    QTimer raise;
    raise.singleShot(1000, this, [=](){
        emit SyncPara();
    });

}

SyncParaFile::~SyncParaFile()
{
    delete ui;
}

void SyncParaFile::SyncParaFiles()
{
    QString category_name = "upgradeSync";
    QString tab_name = "upgrade_para";

    QString parameter_name = "workface_setup";
    QString update_method = "update_method";
    QString parameter_range_min = "update_range_min";
    QString parameter_range_max = "update_range_max";

    QString label = "real_value";
    QStringList range_list;
    QString update_index;
    QString update_index_method;
    QString update_item;
    QString range_min;
    QString range_max;

    if (file_type == TYPE_VERSION) {
        if(access("/others/download/sync.tar.gz", F_OK) == 0)
            unlink("/others/download/sync.tar.gz");
        system("tar -czvf /others/download/sync.tar.gz /userdata/bin /userdata/config");
        update_index_method = this->para->query(category_name, tab_name, update_method, label);
        range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
        update_index = this->para->query(category_name, tab_name, parameter_name, label);
        update_item = range_list[update_index.toInt()];
        range_min = this->para->query(category_name, tab_name, parameter_range_min, label);
        range_max = this->para->query(category_name, tab_name, parameter_range_max, label);
        QJsonObject obj;
        QJsonArray array;
        QString ftp_path;
        obj["sup_no"] = this->comm_acs_thread->cmdManager->address_set->get_local_address();
        if(update_index_method.toInt() == 0){
            array.append(range_min.toInt());
        } else if (update_index_method.toInt() == 2){
            array.append(range_min.toInt());
            array.append(range_max.toInt());
        } else {
            array.append(0);
        }
        obj["range"] = array;
        obj["name"] = QString("sync.tar.gz");
        ftp_path = QString("ftp://");
        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface iface, ifaces)
        {
            //TLOG_INFO("%s:%s\n", iface.name().toUtf8().data(), iface.addressEntries().first().ip().toString().toUtf8().data());
            if(iface.hardwareAddress() != nullptr)
            {
                if(iface.name().compare("eth0") == 0)
                {
                    ftp_path += iface.addressEntries().first().ip().toString();
                    break;
                }
            }
        }
        ftp_path += QString("/others/download/sync.tar.gz");
        obj["path"] = ftp_path;
        obj["version"] = QString("V0.0.1");
        obj["type"] = QString("version");

        QJsonObject info_obj;
        info_obj["info"] = obj;

        //发送给外部
        QJsonObject obj_extern;
        obj_extern["destination"] = array;
        obj_extern["topic"] = QString("update");
        obj_extern["message"] = info_obj;
        QJsonDocument json_ex_doc(obj_extern);
        this->comm_acs_thread->send_socket_msg_to_acs(QString("extern_msg"), json_ex_doc.toJson());

        QTimer raise;
        raise.singleShot(20000, this, [](){
            system("/sbin/reboot");
        });

    }else if (file_type == TYPE_PARAM_DELAY) {
        if(access("/others/download/sync.tar.gz", F_OK) == 0)
            unlink("/others/download/sync.tar.gz");
        system("tar -czvf /others/download/sync.tar.gz /userdata/config");
        update_index_method = this->para->query(category_name, tab_name, update_method, label);
        range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
        update_index = this->para->query(category_name, tab_name, parameter_name, label);
        update_item = range_list[update_index.toInt()];
        range_min = this->para->query(category_name, tab_name, parameter_range_min, label);
        range_max = this->para->query(category_name, tab_name, parameter_range_max, label);
        QJsonObject obj;
        QJsonArray array;
        QString ftp_path;
        obj["sup_no"] = this->comm_acs_thread->cmdManager->address_set->get_local_address();
        if(update_index_method.toInt() == 0){
            array.append(range_min.toInt());
        } else if (update_index_method.toInt() == 2){
            array.append(range_min.toInt());
            array.append(range_max.toInt());
        } else {
            array.append(0);
        }
        obj["range"] = array;
        obj["name"] = QString("sync.tar.gz");
        ftp_path = QString("ftp://");
        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface iface, ifaces)
        {
            if(iface.hardwareAddress() != nullptr)
            {
                if(iface.name().compare("eth0") == 0)
                {
                    ftp_path += iface.addressEntries().first().ip().toString();
                    break;
                }
            }
        }
        ftp_path += QString("/others/download/sync.tar.gz");
        obj["path"] = ftp_path;
        obj["version"] = QString("V0.0.1");
        obj["type"] = QString("param");

        QJsonObject info_obj;
        info_obj["info"] = obj;

        //发送给外部
        QJsonObject obj_extern;
        obj_extern["destination"] = array;
        obj_extern["topic"] = QString("update");
        obj_extern["message"] = info_obj;
        QJsonDocument json_ex_doc(obj_extern);
        this->comm_acs_thread->send_socket_msg_to_acs(QString("extern_msg"), json_ex_doc.toJson());

        QTimer raise;
        raise.singleShot(20000, this, [](){
            system("/sbin/reboot");
        });
    } else if (file_type == TYPE_DEFAULT_PARAM) {
        if(access("/others/restore/rootfs.tar.gz", F_OK) == 0)
        {
            QString cmd = QString("tar -xzvf /others/restore/rootfs.tar.gz -C /");
            system(cmd.toUtf8().data());
            system("sync");
        }
        sync();
        TLOG_INFO("UpdateConfigPage::event reboot");
        system("/sbin/reboot");
    } else if (file_type == TYPE_CONFIG_BACKUP) {
        if(ConfigBackupUtil::backupConfig())
        {
            ui->label->setText("配置备份完成");
            ui->label_2->setText("按取消返回");
        }
        else
        {
            ui->label->setText("配置备份失败");
            ui->label_2->setText("按取消返回");
        }
        operation_done = true;
    } else if (file_type == TYPE_CONFIG_RESTORE) {
        if(ConfigBackupUtil::restoreConfig(restore_file_path))
        {
            ui->label->setText("配置恢复完成，即将重启");
            ui->label_2->setText("恢复完成后自动重启");
            QTimer raise;
            raise.singleShot(3000, this, [](){
                system("/sbin/reboot");
            });
        }
        else
        {
            ui->label->setText("配置恢复失败");
            ui->label_2->setText("按取消返回");
            operation_done = true;
        }
    }

    return;
}

bool SyncParaFile::event(QEvent *event)
{
    if(!operation_done)
        return QWidget::event(event);

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    if(key_event->type() == KeyValue::KEY_RELEASED &&
       (key_event->key() == KeyValue::KEY_CANCEL || key_event->key() == KeyValue::KEY_OK))
    {
        comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
        comm_acs_thread->cmdManager->window = widget_stack->pop();
        close();
        comm_acs_thread->cmdManager->lock_ui_off();
        return true;
    }

    return QWidget::event(event);
}



