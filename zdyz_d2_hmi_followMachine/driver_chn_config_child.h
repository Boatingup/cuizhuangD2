#ifndef DRIVER_CHN_CONFIG_CHILD_H
#define DRIVER_CHN_CONFIG_CHILD_H

#include <QDialog>
#include <QComboBox>
#include "./threads/comm_acs_socket_thread.h"

namespace Ui {
class driver_chn_config_child;
}

class driver_chn_config_child : public QDialog
{
    Q_OBJECT

public:
    explicit driver_chn_config_child(int index, QWidget *parent = nullptr);
    ~driver_chn_config_child();

protected:
    bool event(QEvent *event);

private:
    Ui::driver_chn_config_child *ui;
    QComboBox *get_cur_combox(int row, int column);
    bool combox_changed();
    void update_data(int idx, QComboBox &combobox);
    void reload_driver_config(QString action_name);
    CommAcsSocketThread *comm_acs_thread;
    WidgetStack *widget_stack;

    QComboBox *cur_select_combox;
    int select_flag;
    int row;//行
    int column;//列
    int last_pos;

    int old_combox_pos[9];
    int index;

signals:
    void update_signal();
};

#endif // DRIVER_CHN_CONFIG_CHILD_H
