#ifndef UPDATE_CONFIG_PAGE_H
#define UPDATE_CONFIG_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include "threads/comm_acs_socket_thread.h"
//#include "threads/para_synchronize_file.h"
#include "widget_stack.h"

#include "update_process.h"

namespace Ui {
class UpdateConfigPage;
}

class UpdateConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateConfigPage(int second_level_page, QWidget *parent = nullptr);
    ~UpdateConfigPage();

private:
    Ui::UpdateConfigPage *ui;

private:
    QTableView* tableView;

    QVector<QLabel *> m_menu_list;
private:
    int row;

protected:
    bool event(QEvent *event);

private:
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    Parameters* para;

private:
    int count; //total items in this menu
    int index; //current pointer to a menu item
    int second_page_index;

signals:
    void message_opc_unconnected(int index);

};

#endif // UPDATE_CONFIG_PAGE_H
