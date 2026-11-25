#ifndef DIALOG_WARRANT_H
#define DIALOG_WARRANT_H

#include <QDialog>
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "./Resources/key_value.h"
#include "widget_stack.h"

namespace Ui {
class Dialog_warrant;
}

class Dialog_warrant : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_warrant(QWidget *parent = nullptr);
    ~Dialog_warrant();
protected:
    bool event(QEvent *event);

    void EncData(QByteArray &data);

private:
    Ui::Dialog_warrant *ui;
public:
    CommAcsSocketThread* comm_acs_thread;
private:
    WidgetStack* widget_stack;
    QString str_temp;
};

#endif // DIALOG_WARRANT_H
