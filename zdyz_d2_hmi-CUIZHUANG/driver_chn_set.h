#ifndef DRIVER_CHN_SET_H
#define DRIVER_CHN_SET_H

#include <QDialog>
#include <QComboBox>
#include "./threads/comm_acs_socket_thread.h"
namespace Ui {
class driver_chn_set;
}

class driver_chn_set : public QDialog
{
    Q_OBJECT

public:
    explicit driver_chn_set(QWidget *parent = nullptr);
    ~driver_chn_set();

    CommAcsSocketThread *comm_acs_thread;
    WidgetStack *widget_stack;
protected:
    bool event(QEvent *event);
private:
    Ui::driver_chn_set *ui;

    int flag;
    int recover;
signals:
    void save_signal(int flag);
};

#endif // DRIVER_CHN_SET_H
