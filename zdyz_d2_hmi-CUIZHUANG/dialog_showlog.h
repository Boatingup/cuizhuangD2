#ifndef DIALOG_SHOWLOG_H
#define DIALOG_SHOWLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QDialog>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"

namespace Ui {
class Dialog_showLog;
}

class Dialog_showLog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_showLog(QWidget *parent = nullptr);
    ~Dialog_showLog();
    int getLog(const char *,QString &str);
protected:
    bool event(QEvent *event);

private:
    Ui::Dialog_showLog *ui;
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
};

#endif // DIALOG_SHOWLOG_H
