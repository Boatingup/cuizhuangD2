#ifndef SHOW_VERSION_H
#define SHOW_VERSION_H

#include <QDialog>
#include "Resources/parameters.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
#include "threads/comm_acs_socket_thread.h"

#define HMI_VERSION "V1.2.14.261026"

namespace Ui {
class show_version;
}

class show_version : public QDialog
{
    Q_OBJECT

public:
    explicit show_version(QWidget *parent = nullptr);
    ~show_version();
    QString get_hmi_version();
    QString get_kernel_version();
protected:
    bool event(QEvent *event);
private:
    Ui::show_version *ui;
    Parameters* para;
private:
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    CmdManager* cmdManager;


};

#endif // SHOW_VERSION_H
