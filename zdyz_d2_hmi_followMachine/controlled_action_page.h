#ifndef CONTROLLED_ACTION_PAGE_H
#define CONTROLLED_ACTION_PAGE_H

#include <QWidget>

#include "Resources/action_register.h"
#include "Resources/address_set.h"
#include "Resources/command.h"
#include "Resources/parameters.h"
#include "Resources/sensor.h"
#include "Resources/system_state.h"
#include "Resources/action_register.h"
#include "Resources/key_value.h"

#include "threads/comm_acs_socket_thread.h"
#include <QMovie>

namespace Ui {
class ControlledActionPage;
}

class ControlledActionPage : public QWidget
{
    Q_OBJECT

public:
     ControlledActionPage(QWidget *mainpage, int page, QString trip, QWidget *parent = nullptr);
    ~ControlledActionPage();

     CommAcsSocketThread* comm_acs_thread;
     void runGif();
     QTimer * timer_close;
     ActionRegister* action_register;

public slots:
    void update_action_descriptor(QString& action_descriptor);
    void update_follow_direction(int dir);                      // 0 for stop, 1 for up, 2 for down


    void update_follow_step(QString step);
    //void close_push();

private:
    QWidget* mainpage;

protected:
    bool event(QEvent *event);

private:
    AddressSet* address_set;
    QMovie *movie = nullptr;
private:
    Ui::ControlledActionPage *ui;
    SystemState *state = nullptr;

};

#endif // CONTROLLED_ACTION_PAGE_H
