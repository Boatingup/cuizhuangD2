#ifndef MENUPAGE_H
#define MENUPAGE_H

#include <QWidget>
#include <QTimer>

#include "../threads/comm_acs_socket_thread.h"
#include "widget_stack.h"

#include "../Resources/parameters.h"

namespace Ui {
class menuPage;
}
class QAction;
class menuPage : public QWidget
{
    Q_OBJECT

public:
    explicit menuPage(QWidget *parent = nullptr);
    ~menuPage();
    void toolBtnInit();

public:
    void refresh(int, int) const;


signals:
    void update_interface(int);

public slots:
    void timer_closecb(void);

private:
    Ui::menuPage *ui;

protected:
    bool event(QEvent *event);

private:
    int label_index; //current index of functions

public:
    CommAcsSocketThread* comm_acs_thread;
private:
    WidgetStack* widget_stack;

private:
    QTimer* timer_back_to_mainpage;

private:
    Parameters* para;

private:
    int main_control_time; //ms

};

#endif // MENUPAGE_H
