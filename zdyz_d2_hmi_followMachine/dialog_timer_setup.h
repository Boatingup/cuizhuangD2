#ifndef DIALOG_TIMER_SETUP_H
#define DIALOG_TIMER_SETUP_H
#include "Resources/key_value.h"
#include "threads/comm_acs_socket_thread.h"
#include "./threads/para_synchronize_thread.h"
#include <QDialog>
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
namespace Ui {
class Dialog_timer_setup;
}

class Dialog_timer_setup : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_timer_setup(QWidget *parent = nullptr);
    ~Dialog_timer_setup();
protected:
    bool event(QEvent *event);

private:
    Ui::Dialog_timer_setup *ui;
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    ParaSynchronizeThread* pst;
    QList<int> Sections;
    void init_sections();
    int section_index = 0;
    QDateTime datetime;
    QDateTime datetime_real;
};

#endif // DIALOG_TIMER_SETUP_H
