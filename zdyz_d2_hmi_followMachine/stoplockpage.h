#ifndef STOPLOCKPAGE_H
#define STOPLOCKPAGE_H

#include <QWidget>
#include <QLabel>
#include "./Resources/address_set.h"



#include "widget_stack.h"

namespace Ui {
class StopLockpage;
}

class StopLockpage : public QWidget
{
    Q_OBJECT

public:
    explicit StopLockpage(QWidget *parent = nullptr);

    StopLockpage(QString stop_lock, QString number, QWidget *parent = nullptr);

    ~StopLockpage();


public:
    void update(QString stop_lock, QString number);

    AddressSet* address_set;

private:
    Ui::StopLockpage *ui;
private:
    WidgetStack* widget_stack;

private:
    QLabel* label_human_lock;
};

#endif // STOPLOCKPAGE_H
