#ifndef NET_PHONE_PAGE_H
#define NET_PHONE_PAGE_H

#include <QWidget>
#include <QDialog>
#include "Resources/parameters.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
#include "threads/comm_acs_socket_thread.h"

namespace Ui {
class NetPhonePage;
}

class NetPhonePage : public QWidget
{
    Q_OBJECT

public:
    explicit NetPhonePage(QWidget *parent = nullptr);
    ~NetPhonePage();
    QTimer * timer_close;

    void set_occuried(int occur_io);
private:
    Ui::NetPhonePage *ui;
    Parameters* para;
private:
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
};

#endif // NET_PHONE_PAGE_H
