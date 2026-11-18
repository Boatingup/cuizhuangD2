#ifndef SYSTEM_CONFIG_PAGE_H
#define SYSTEM_CONFIG_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"


namespace Ui {
class systemConfigPage;
}

class SystemConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit SystemConfigPage(int second_level_page,QWidget *parent = nullptr);
    ~SystemConfigPage();

private:
    Ui::systemConfigPage *ui;

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

private:
    int count; //total items in this menu
    int index; //current pointer to a menu item
    int second_page_index;
    QString pass_enter_testApp;
};

#endif // SYSTEM_CONFIG_PAGE_H
