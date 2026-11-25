#ifndef DIALOG_HELP_H
#define DIALOG_HELP_H

#include <QDialog>
#include <QFile>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"
#include "Resources/key_value.h"
#include <QEvent>
#include <QKeyEvent>

namespace Ui {
class Dialog_help;
}

class Dialog_help : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_help(QWidget *parent = nullptr);
    int GetStylePath();
    ~Dialog_help();
protected:
    bool event(QEvent *event);
private:
    Ui::Dialog_help *ui;
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    Sensor* sensor;
};

#endif // DIALOG_HELP_H
