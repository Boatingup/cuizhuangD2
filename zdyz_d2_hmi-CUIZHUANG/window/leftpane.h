#ifndef LEFTPANE_H
#define LEFTPANE_H

#include <QWidget>
#include <QModelIndex>
#include <QTableWidgetItem>
#include <QMap>

#include "../Resources/parameters.h"
#include "../threads/comm_acs_socket_thread.h"

#include <QEvent>

#include "widget_stack.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"



namespace Ui {
class leftPane;
}
class QStandardItemModel;
class leftPane : public QWidget
{
    Q_OBJECT

public:
    //explicit leftPane(QWidget *parent = nullptr);
    leftPane(QString& category, QWidget *parent = nullptr);
    ~leftPane();
public:
     struct tableViewArr{
        QTableView* tableView_left;
        QTableView* tableView_right;
     };

     struct model_index{
         QString index;
         QStandardItemModel* model_left;
         QStandardItemModel* model_right;
         int total;
         struct tableViewArr tab_array;
     };

public:
    void load_parameters(int tab);
    void load_parameters(int tab,int index, bool flag);
    void set_array_tab(struct tableViewArr arr, QStandardItemModel *model);
protected:
     bool event(QEvent *event);
private:
    Ui::leftPane *ui;
    QStandardItemModel *m_pModle;
    QTabWidget *tabWidget;
    QTimer *timer;

public slots:
    void timer_cb(void);
private:
     Parameters* para;

private:
     QList<QStringList> para_list;
     QList<QList <int>> index_list;

     QList<tableViewArr> table_view_arr_list;

     int total_load = 0;

     QList <model_index>  model_list;

     bool InRoList();

     int  FindModelIndex(int index,int &index_num);
     int  FindNameByIndex(QString & name,int & index);// in readonly list

     QList <QList <int>> list_index;

signals:
     void signalloadframe(int size);

private slots:
    //void itemDoubleClick(QModelIndex index);

public slots:
    void slotloadframe(int size);

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

    //QMap <int,QStandardItemModel*>  model_list_map;

private:
    WidgetStack* widget_stack;

};

#endif // LEFTPANE_H
