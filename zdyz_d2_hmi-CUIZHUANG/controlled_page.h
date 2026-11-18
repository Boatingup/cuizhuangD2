#ifndef CONTROLLED_PAGE_H
#define CONTROLLED_PAGE_H

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
class controlled_page;
}

class controlled_page : public QWidget
{
    Q_OBJECT

public:
    explicit controlled_page(QWidget *mainpage, QWidget *parent = nullptr);
    ~controlled_page();
    CommAcsSocketThread* comm_acs_thread;
    void run_gif(QString file_name);
    QTimer * timer_close;
    ActionRegister* action_register;

public slots:
   void update_action_descriptor(QString &action_descriptor, QString &gif_info);
   void update_follow_direction(int dir);                      // 0 for stop, 1 for up, 2 for down
   void update_follow_step(QString step);

   void update_sensor_info();
   void update_sensor_data(unsigned int view);
private:
   QWidget* mainpage;

protected:
   bool event(QEvent *event);

private:
   AddressSet* address_set;
   QMovie *movie = nullptr;
private:
   SystemState *state = nullptr;
    Ui::controlled_page *ui;
    Sensor *sensor;
};

#endif // CONTROLLED_PAGE_H
