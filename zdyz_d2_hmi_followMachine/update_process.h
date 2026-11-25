#ifndef UPDATE_PROCESS_H
#define UPDATE_PROCESS_H

#include <QWidget>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"

namespace Ui {
class update_process;
}

class update_process : public QWidget
{
    Q_OBJECT

public:
    explicit update_process(QString data = "none", QWidget *parent = 0);
    ~update_process();

public slots:
    void process_update(double cur, double total);
    void status_update(QString status);
    void file_set(QString name);

private:
    Ui::update_process *ui;
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;

protected:
    bool event(QEvent *event);
};

#endif // UPDATE_PROCESS_H
