#ifndef SHOW_RUNERROR_H
#define SHOW_RUNERROR_H

#include <QDialog>
#include "Resources/key_value.h"
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"

#include "Resources/parameters.h"
#include "./Resources/parameters.h"

namespace Ui {
class show_runerror;
}

class show_runerror : public QDialog
{
    Q_OBJECT

public:
    explicit show_runerror(QWidget *parent = nullptr);
    ~show_runerror();
    int GetStylePath(QString &css);

    CommAcsSocketThread* comm_acs_thread;
protected:
    bool event(QEvent *event);

private:
    Ui::show_runerror *ui;
    Parameters* para;
    WidgetStack* widget_stack;
private:
    QMap <QString,int> map_error;
    QStringList range_list;

public slots:
    void update_system_fault(QString,QString);
    void show_error(QString str,QString info);


};

#endif // SHOW_RUNERROR_H
