#ifndef ACTIONCONFIGURE_H
#define ACTIONCONFIGURE_H

#include <QWidget>
#include <QModelIndex>
#include <QTableWidgetItem>
#include <QMap>

#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"

#include <QEvent>

#include "widget_stack.h"

namespace Ui {
class ActionConfigure;
}
class QStandardItemModel;

class ActionConfigure : public QWidget
{
    Q_OBJECT

public:
    //explicit leftPane(QWidget *parent = nullptr);
    ActionConfigure(QString& category, QWidget *parent = nullptr);
    ~ActionConfigure();
    void load_actions();


public:
     struct tableViewArr{
        QTableView* tableView_left;
        QTableView* tableView_right;
     };

public:
    ActionRegister* action_register;
    //void load_parameters(int tab);
    void load_actions(int tab);
    void load_actions(int tab,int index);
protected:
     bool event(QEvent *event);
     //void paintEvent(QPaintEvent *e);
private:
    QStandardItemModel *m_pModle;

    QTabWidget *tabWidget;


private:
     Parameters* para;

private:
     QList<QStringList> para_list;

     QList<tableViewArr> table_view_arr_list;


private slots:
    //void itemDoubleClick(QModelIndex index);

private:
    // changed to class Parameters
    //QMap<QString, QString> map; //存放分类名和中文显示字符的对照表，如"system"--"系统参数"

private:
    int row;
    int column;
    int index;

    int tab;

    int para_list_size;

private:
    CommAcsSocketThread* comm_acs_thread;
private:
    QString category;

private:
    WidgetStack* widget_stack;


private:
    Ui::ActionConfigure *ui;

};

#endif // ACTIONCONFIGURE_H
