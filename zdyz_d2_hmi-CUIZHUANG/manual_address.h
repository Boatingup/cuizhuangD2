#ifndef MANUAL_ADDRESS_H
#define MANUAL_ADDRESS_H

#include <QWidget>
#include "./Resources/address_set.h"
#include "./threads/comm_acs_socket_thread.h"

#include "widget_stack.h"

namespace Ui {
class ManualAddress;
}

class ManualAddress : public QWidget
{
    Q_OBJECT

public:
    explicit ManualAddress(QWidget *parent = nullptr);
    ~ManualAddress();

protected:
    bool event(QEvent *event);
private:
    Ui::ManualAddress *ui;

private:
    AddressSet* address_set;

    CommAcsSocketThread* comm_acs_thread;

public slots:
    void timer_cb(void);

public:
    QTimer * timer_close;

private:
    QString str_temp;

private:
    bool first_time_to_input;

private:
    WidgetStack* widget_stack;

private:
    int is_auto_addressing;

private:
    QTimer *timer;
};

#endif // MANUAL_ADDRESS_H
