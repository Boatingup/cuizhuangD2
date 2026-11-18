#ifndef CONFIG_BACKUP_RESTORE_H
#define CONFIG_BACKUP_RESTORE_H

#include <QWidget>
#include <QLabel>
#include <QStringList>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"

namespace ConfigBackupUtil {
    static const char *BACKUP_DIR = "/others/download";
    static const int MAX_BACKUPS = 5;

    QStringList listBackupFileNames();
    QString backupDisplayName(const QString &fileName);
    bool backupConfig();
    bool restoreConfig(const QString &backupFileName);
}

class ConfigRestorePage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigRestorePage(QWidget *parent = nullptr);
    ~ConfigRestorePage();

protected:
    bool event(QEvent *event);

private:
    QVector<QLabel *> m_menu_list;
    QStringList m_backup_files;
    int row;
    int count;
    CommAcsSocketThread *comm_acs_thread;
    WidgetStack *widget_stack;
};

#endif // CONFIG_BACKUP_RESTORE_H
