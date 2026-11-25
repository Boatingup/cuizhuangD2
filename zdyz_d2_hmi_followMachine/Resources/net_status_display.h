#ifndef NET_STATUS_DISPLAY_H
#define NET_STATUS_DISPLAY_H

#include <QWidget>

namespace Ui {
class net_status_display;
}

class net_status_display : public QWidget
{
    Q_OBJECT

public:
    explicit net_status_display(QWidget *parent = 0);
    ~net_status_display();

private:
    Ui::net_status_display *ui;
};

#endif // NET_STATUS_DISPLAY_H
