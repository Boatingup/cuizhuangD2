#ifndef FAULT_DISPLAY_H
#define FAULT_DISPLAY_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QEvent>

#include "widget_stack.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "fault_diagnosis_diaplsy.h"
#include "./Resources/key_value.h"

namespace Ui {
class fault_display;
}

class fault_display : public QWidget
{
    Q_OBJECT

public:
    explicit fault_display(QString& category, QWidget *parent = nullptr);
    //explicit fault_display(QWidget *parent = nullptr);
    ~fault_display();

    void load_fault();


public:
     struct tableViewArr{
        QTableView* tableView_left;
        QTableView* tableView_right;
     };

public:
    ActionRegister* action_register;
    void fault_list_update();

    void load_actions(int tab);
    void load_actions(int tab,int index);
protected:
     bool event(QEvent *event);
private:
    QStandardItemModel *m_pModle;
    QTabWidget *tabWidget;
    Parameters *para;

    QList<QStringList> para_list;
    QList<tableViewArr> table_view_arr_list;

    QList<QPair<QString, FAULT_INFO_S>> fault_list;

private:
    int row;
    int column;
    int index;
    int tab;
    int fault_list_size;

private:
    CommAcsSocketThread* comm_acs_thread;
private:
    QString category;

private:
    WidgetStack* widget_stack;
private:
    Ui::fault_display *ui;
};

#endif // FAULT_DISPLAY_H
