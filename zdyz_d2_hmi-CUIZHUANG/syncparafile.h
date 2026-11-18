#ifndef SYNCPARAFILE_H
#define SYNCPARAFILE_H

#include <QWidget>
#include "update_config_page.h"
#include <QTimer>

#include "threads/comm_acs_socket_thread.h"
//#include "threads/para_synchronize_file.h"
#include "widget_stack.h"

typedef enum
{
    TYPE_PARAM,
    TYPE_DRIVER,
    TYPE_CONNECT_BOARD,
    TYPE_DEFAULT_PARAM,
    TYPE_VERSION,
    TYPE_PARAM_DELAY,
    TYPE_CONFIG_BACKUP,
    TYPE_CONFIG_RESTORE
}FILE_TYPE_E;

namespace Ui {
class SyncParaFile;
}

class SyncParaFile : public QWidget
{
    Q_OBJECT

public:
    explicit SyncParaFile(FILE_TYPE_E type, QWidget *parent = nullptr, const QString &file_path = QString());
    ~SyncParaFile();

private:
    QTimer *timer;
    FILE_TYPE_E file_type;
    QString restore_file_path;
    bool operation_done;
signals:
    void SyncPara();

public slots:
    void SyncParaFiles();

protected:
    bool event(QEvent *event);

public:
    UpdateConfigPage *update_config_page;
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    Parameters* para;

private:
    Ui::SyncParaFile *ui;
};

#endif // SYNCPARAFILE_H
