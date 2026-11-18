#ifndef AUXILIARYMENU_H
#define AUXILIARYMENU_H

#include <QDialog>
#include <QTextCodec>
#include <QTableView>

#include "./threads/keyboardreadthread.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "./threads/para_synchronize_thread.h"

#include "widget_stack.h"

namespace Ui {
class AuxiliaryMenu;
}

class AuxiliaryMenu : public QDialog
{
    Q_OBJECT

public:
    explicit AuxiliaryMenu(QWidget *parent = nullptr);
    ~AuxiliaryMenu();

protected:
    bool event(QEvent *event);

public:
    CommAcsSocketThread* comm_acs_thread;
    ActionRegister* action_register;
    SystemState* ss;

private:
    WidgetStack* widget_stack;
    QTableView* tableView;
    QTableView* right_tableView;
    int row;
    int index;
    int count;
    int exact_size = 0;
    int page_num;
    int page_index;
signals:
    void change_user();

private:
    Ui::AuxiliaryMenu *ui;
};

#endif // AUXILIARYMENU_H
