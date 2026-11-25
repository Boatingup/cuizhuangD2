#ifndef PUMPSTATION_MONITOR_H
#define PUMPSTATION_MONITOR_H

#include <QWidget>

namespace Ui {
class pumpstation_monitor;
}

class pumpstation_monitor : public QWidget
{
    Q_OBJECT

public:
    explicit pumpstation_monitor(QWidget *parent = 0);
    ~pumpstation_monitor();

private:
    Ui::pumpstation_monitor *ui;
};

#endif // PUMPSTATION_MONITOR_H
