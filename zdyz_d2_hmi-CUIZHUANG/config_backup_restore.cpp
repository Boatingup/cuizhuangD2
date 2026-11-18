#include "config_backup_restore.h"
#include "Resources/key_value.h"
#include "syncparafile.h"
#include "log_head.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRect>
#include <QFont>
#include <QPixmap>
#include <unistd.h>

namespace ConfigBackupUtil {

QStringList listBackupFileNames()
{
    QDir dir(BACKUP_DIR);
    if(!dir.exists())
        return QStringList();

    QStringList files = dir.entryList(QStringList() << "config_*.tar.gz", QDir::Files, QDir::Time);
    return files;
}

QString backupDisplayName(const QString &fileName)
{
    QString base = fileName;
    if(base.startsWith("config_"))
        base = base.mid(7);
    if(base.endsWith(".tar.gz"))
        base.chop(7);

    QDateTime dateTime = QDateTime::fromString(base, "yyyyMMdd_HHmmss");
    if(dateTime.isValid())
        return dateTime.toString("yyyy-MM-dd HH:mm:ss");
    return fileName;
}

static void ensureBackupDir()
{
    if(access(BACKUP_DIR, F_OK) != 0)
        system("mkdir -p /others/download");
}

static void pruneOldBackups()
{
    QDir dir(BACKUP_DIR);
    if(!dir.exists())
        return;

    QFileInfoList files = dir.entryInfoList(QStringList() << "config_*.tar.gz", QDir::Files, QDir::Time);
    while(files.size() >= MAX_BACKUPS)
    {
        QFile::remove(files.last().absoluteFilePath());
        files.removeLast();
    }
}

bool backupConfig()
{
    ensureBackupDir();
    pruneOldBackups();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString backupPath = QString("%1/config_%2.tar.gz").arg(BACKUP_DIR).arg(timestamp);
    QString cmd = QString("tar -czvf %1 -C /userdata config").arg(backupPath);
    int ret = system(cmd.toUtf8().constData());
    sync();
    TLOG_INFO("config backup: %s, ret=%d\n", backupPath.toUtf8().constData(), ret);
    return ret == 0;
}

bool restoreConfig(const QString &backupFileName)
{
    QString backupPath = QString("%1/%2").arg(BACKUP_DIR).arg(backupFileName);
    if(access(backupPath.toUtf8().constData(), F_OK) != 0)
    {
        TLOG_INFO("config restore failed, file not found: %s\n", backupPath.toUtf8().constData());
        return false;
    }

    QString cmd = QString("tar -xzvf %1 -C /userdata").arg(backupPath);
    int ret = system(cmd.toUtf8().constData());
    sync();
    TLOG_INFO("config restore: %s, ret=%d\n", backupPath.toUtf8().constData(), ret);
    return ret == 0;
}

} // namespace ConfigBackupUtil

ConfigRestorePage::ConfigRestorePage(QWidget *parent) :
    QWidget(parent),
    row(0),
    count(0),
    comm_acs_thread(CommAcsSocketThread::getInstance()),
    widget_stack(WidgetStack::getInstance())
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(800, 480);
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    QLabel *background = new QLabel(this);
    background->setGeometry(QRect(0, 0, 800, 480));
    background->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update/update_background.png")));
    background->lower();

    m_backup_files = ConfigBackupUtil::listBackupFileNames();
    count = m_backup_files.size();

    QFont hintFont;
    hintFont.setFamily(QString::fromUtf8("YouSheBiaoTiHei"));
    hintFont.setPointSize(24);

    QLabel *subtitle = new QLabel(this);
    subtitle->setGeometry(QRect(0, 52, 800, 30));
    subtitle->setFont(hintFont);
    subtitle->setStyleSheet(QString::fromUtf8("color: rgb(180, 220, 255);"));
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setText(count > 0 ? QString::fromUtf8("请选择备份文件") : QString::fromUtf8("暂无备份文件"));

    QFont footerFont;
    footerFont.setFamily(QString::fromUtf8("YouSheBiaoTiHei"));
    footerFont.setPointSize(26);

    QLabel *footer = new QLabel(this);
    footer->setGeometry(QRect(0, 420, 800, 40));
    footer->setFont(footerFont);
    footer->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
    footer->setAlignment(Qt::AlignCenter);
    footer->setText(count > 0 ? QString::fromUtf8("按确认恢复    按取消返回")
                              : QString::fromUtf8("按取消返回"));

    if(count == 0)
        return;

    for(int i = 0; i < count; i++)
    {
        QLabel *label = new QLabel(this);
        label->setGeometry(QRect(0, 80 + 55 * i, 775, 55));
        label->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
        label->setAlignment(Qt::AlignRight);
        m_menu_list.push_back(label);

        QLabel *label_text = new QLabel(this);
        label_text->setGeometry(QRect(90, 80 + 55 * i, 660, 55));
        QFont font;
        font.setFamily(QString::fromUtf8("YouSheBiaoTiHei"));
        font.setPointSize(28);
        label_text->setFont(font);
        label_text->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_text->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label_text->setText(ConfigBackupUtil::backupDisplayName(m_backup_files.at(i)));
        m_menu_list.push_back(label_text);
    }

    m_menu_list.at(0)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));
}

ConfigRestorePage::~ConfigRestorePage()
{
    for(int i = 0; i < m_menu_list.size(); i++)
        delete m_menu_list.at(i);
}

bool ConfigRestorePage::event(QEvent *event)
{
    if(count == 0)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if(key_event->type() == KeyValue::KEY_RELEASED &&
           (key_event->key() == KeyValue::KEY_CANCEL || key_event->key() == KeyValue::KEY_OK))
        {
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            QWidget *prev_page = widget_stack->pop();
            prev_page->show();
            comm_acs_thread->cmdManager->window = prev_page;
            close();
            comm_acs_thread->cmdManager->lock_ui_off();
            return true;
        }
        return QWidget::event(event);
    }

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_RELEASED:
        switch(key_event->key())
        {
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(row > 0)
            {
                m_menu_list.at(row * 2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
                m_menu_list.at((row - 1) * 2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));
                --row;
            }
            comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(row < count - 1)
            {
                m_menu_list.at(row * 2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
                m_menu_list.at((row + 1) * 2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));
                ++row;
            }
            comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            {
                SyncParaFile *sync_param = new SyncParaFile(TYPE_CONFIG_RESTORE, nullptr, m_backup_files.at(row));
                sync_param->show();
                widget_stack->push(this);
                comm_acs_thread->cmdManager->window = sync_param;
            }
            comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
        {
            QWidget *prev_page = widget_stack->pop();
            prev_page->show();
            comm_acs_thread->cmdManager->window = prev_page;
            close();
            TLOG_INFO("config restore page: key cancel");
            comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }

        default:
            comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;

    default:
        break;
    }

    return QWidget::event(event);
}
