#include "actionconfigure.h"
#include "ui_actionconfigure.h"
#include <QDebug>
#include <QFile>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTableView>
#include <QDateTime>

#include <QString>
#include <QStringList>
#include <QList>

#include <QEvent>
#include <QKeyEvent>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "./Resources/key_value.h"

#include "parameter_config.h"
#include "action_set.h"

#include "./window/menupage.h"
#include "./Resources/error_code.h"

#include <QPainter>

/*********************************************************
 * @file      : actionconfigure.cpp
 * @brief     : already abandon
 * @author    : fxs
 * @date      : 2026-07-28
 * @version   : V1.0
 *********************************************************/

ActionConfigure::ActionConfigure(QString& category, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionConfigure)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(800, 480);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();

    this->category = category;
    //connect(this->comm_acs_thread->control_timer, SIGNAL(timeout()), this->comm_acs_thread, SLOT(closewindow()));
    this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    QFile file(":/qss/actionconfig.qss");
    //QFile file(":/qss/stoplockpage.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        //LOG(INFO) << ss.toLocal8Bit().data();
        this->setStyleSheet(ss);
        file.close();
    }

    this->action_register = ActionRegister::getInstance();
    this->row = -1;  //光标所在行,-1是为了按向下按键时+1后为0，对应list中下标为0
    this->column = 0;//光标所在列， 0为左列，1为右列
    this->index  = 0;//在subblock标签下中对应参数的索引
    this->tab = 0; //subblock的对应标签

    this->para = Parameters::getInstance();

    this->tabWidget = new QTabWidget(this);

    tabWidget->move(0, 40);
    tabWidget->setGeometry(0, 40, 800, 440);

    QTableView* tableView_left;
    QTableView* tableView_right;

    TLOG_INFO("para_list.size() : %d", para_list.size());
        QWidget *tab = new QWidget();
        QHBoxLayout* hlayout = new QHBoxLayout();
        hlayout->setMargin(0);
        hlayout->setSpacing(0);

        struct tableViewArr table_view_arr;
        tableView_left = table_view_arr.tableView_left = new QTableView();
        tableView_right = table_view_arr.tableView_right = new QTableView();

        table_view_arr_list.append(table_view_arr);

        //设置左半侧tableview
        tableView_left->setShowGrid(false); //不显示格子线
        tableView_left->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_left->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
        tableView_left->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        //设置右半侧tableview
        tableView_right->setShowGrid(false); //不显示格子线
        tableView_right->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_right->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
        tableView_right->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        hlayout->addWidget(tableView_left);
        hlayout->addWidget(tableView_right);

        tab->setLayout(hlayout);
        tabWidget->addTab(tab, "动作列表");


    load_actions();

    QFont font = ui->label_category->font();
    font.setBold(true);
    ui->label_category->setFont(font); // 字体加粗
    ui->label_category->setText(this->para->category_name.value(category));
}

void ActionConfigure::load_actions()
{
        TLOG_INFO("in load action ()");
        int tab = 0;

        this->action_register->reload();

        struct tableViewArr arr = this->table_view_arr_list.at(tab);

        this->para_list_size = this->action_register->action_list.size();
        TLOG_INFO("0000000000000000000000000000000000000 para_list_size:%d\n", para_list_size);

        QStandardItemModel* model = new QStandardItemModel();
        model->setColumnCount(2);
        model->setHeaderData(0, Qt::Horizontal, QString("名称"));
        model->setHeaderData(1, Qt::Horizontal, QString("数值"));

        arr.tableView_left->setModel(model);
        //设置宽度要在设置了model之后调用
        arr.tableView_left->setColumnWidth(0, 140);
        arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        arr.tableView_left->horizontalHeader()->setVisible(false);
        //表中有被选中的行时，表头不变色
        arr.tableView_left->horizontalHeader()->setHighlightSections(false);

        QFont font;

        QList <QPair<QString,Action_info>>::const_iterator it = action_register->action_list.constBegin();
        int i = 0;
        int row = 0;
        TLOG_INFO("11111\n");
        if(action_register->action_list.size() == 0)
        {
            TLOG_INFO("action.xml is null");
            return;
        }
        while (it != action_register->action_list.constEnd()) {
            //qDebug() << it.key() << ": " << it.value();
            model->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
            model->setItem(row, 1, new QStandardItem(it->second.action_name));

            model->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
            model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

            font = model->item(row, 0)->font();
            font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加

            model->item(row, 0)->setFont(font);
            model->item(row, 1)->setFont(font);

            i+=2;
            if(i > action_register->action_list.size() - 1)
            {
                break;
            }
            row++;
            it += 2;
        }

        QStandardItemModel* model_r = new QStandardItemModel();
        model_r->setColumnCount(2);
        model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
        model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));
        arr.tableView_right->setModel(model_r);
        //设置宽度要在设置了model之后调用

        arr.tableView_right->setColumnWidth(0, 140);
        arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        arr.tableView_right->horizontalHeader()->setVisible(false);
        //表中有被选中的行时，表头不变色
        arr.tableView_right->horizontalHeader()->setHighlightSections(false);

        it = action_register->action_list.constBegin();
        it++;
        i = 1;
        row = 0;
        while (it != action_register->action_list.constEnd()) {
            //qDebug() << it.key() << ": " << it.value();
            model_r->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
            model_r->setItem(row, 1, new QStandardItem(it->second.action_name));

            model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
            model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);

            font = model_r->item(row, 0)->font();
            font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加

            model_r->item(row, 0)->setFont(font);
            model_r->item(row, 1)->setFont(font);
            i+=2;
            //row++;
            if(i > action_register->action_list.size() - 1)
            {
                break;
            }
            row++;
            it += 2;
        }

        if(action_register->action_list.size() % 2 == 1)
        {
            int last_row = action_register->action_list.size() / 2;
            model_r->setItem(last_row, 0, new QStandardItem(" ")); //设置tablebiew中的文字内容
            model_r->setItem(last_row, 1, new QStandardItem(" "));

            model_r->item(last_row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
            model_r->item(last_row, 1)->setTextAlignment(Qt::AlignCenter);

            font = model_r->item(last_row, 0)->font();
            font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加

            model_r->item(last_row, 0)->setFont(font);
            model_r->item(last_row, 1)->setFont(font);
        }
        arr.tableView_left->clearSelection();
        arr.tableView_right->clearSelection();

}

void ActionConfigure::load_actions(int tab,int row_num)
{
    tab = 0;
    struct tableViewArr arr = this->table_view_arr_list.at(tab);
    this->para_list_size = this->action_register->action_list.size();

    QStandardItemModel* model = new QStandardItemModel();
    model->setColumnCount(2);
    model->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_left->setModel(model);
    //设置宽度要在设置了model之后调用
    arr.tableView_left->setColumnWidth(0, 140);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_left->horizontalHeader()->setVisible(false);
    //表中有被选中的行时，表头不变色
    arr.tableView_left->horizontalHeader()->setHighlightSections(false);

    QFont font;

    QList<QPair<QString,Action_info>>::const_iterator it = action_register->action_list.constBegin();
    int i = 0;
    int row = 0;
    TLOG_INFO("11111\n");
    while (it != action_register->action_list.constEnd()) {
        //qDebug() << it.key() << ": " << it.value();
        model->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
        model->setItem(row, 1, new QStandardItem(it->second.action_name));

        model->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model->item(row, 0)->font();
        font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加

        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);

        i+=2;
        //row++;
        if(i > action_register->action_list.size() - 1)
        {
            break;
        }
        row++;
        it += 2;
    }

    QStandardItemModel* model_r = new QStandardItemModel();
    model_r->setColumnCount(2);
    model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));
    arr.tableView_right->setModel(model_r);
    //设置宽度要在设置了model之后调用

    arr.tableView_right->setColumnWidth(0, 140);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

   // arr.tableView_right->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    arr.tableView_right->horizontalHeader()->setVisible(false);
    //表中有被选中的行时，表头不变色
    arr.tableView_right->horizontalHeader()->setHighlightSections(false);

    it = action_register->action_list.constBegin();
    it++;
    i = 1;
    row = 0;
    while (it != action_register->action_list.constEnd()) {
        //qDebug() << it.key() << ": " << it.value();
        model_r->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
        model_r->setItem(row, 1, new QStandardItem(it->second.action_name));

        model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model_r->item(row, 0)->font();
        font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加

        model_r->item(row, 0)->setFont(font);
        model_r->item(row, 1)->setFont(font);
        i+=2;
        //row++;
        if(i > action_register->action_list.size() - 1)
        {
            break;
        }
        row++;
        it += 2;
    }
    arr.tableView_left->clearSelection();
    arr.tableView_right->clearSelection();

    if(row_num % 2 == 0)
    {
        arr.tableView_left->selectRow(row_num / 2);
    }

    if(row_num %2 != 0)
    {
        arr.tableView_right->selectRow(row_num / 2);
    }

}

ActionConfigure::~ActionConfigure()
{
    delete ui;
}

bool ActionConfigure::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->column = 1;
            if((para_list_size %2) == 1)
            {
                if(this->row == (para_list_size/2))
                {
                     this->column = 0;
                }
                else {
                      this->table_view_arr_list[tab].tableView_left->clearSelection();
                    this->table_view_arr_list[tab].tableView_right->selectRow(this->row);
                }
            } else {
                this->table_view_arr_list[tab].tableView_left->clearSelection();
                this->table_view_arr_list[tab].tableView_right->selectRow(this->row);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->column = 0;
            this->table_view_arr_list[tab].tableView_right->clearSelection();
            this->table_view_arr_list[tab].tableView_left->selectRow(this->row);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->column == 0){ //in the left table view
                if(this->row > 0){
                    this->table_view_arr_list[tab].tableView_left->selectRow(--this->row);
                    this->table_view_arr_list[tab].tableView_right->clearSelection();
                }
            }else{  // in the right table view
                if(this->row > 0){
                    this->table_view_arr_list[tab].tableView_right->selectRow(--this->row);
                    this->table_view_arr_list[tab].tableView_left->clearSelection();
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->column == 0){ //in the left table view
                if(this->row < ((this->para_list_size % 2) ? (this->para_list_size + 1) / 2 : (this->para_list_size / 2)) - 1){
                    this->table_view_arr_list[tab].tableView_left->selectRow(++this->row);
                    this->table_view_arr_list[tab].tableView_right->clearSelection();
                }
            }else{ //in the right table view
                if(this->row < ((this->para_list_size % 2) ? (this->para_list_size + 1) / 2 : (this->para_list_size / 2)) - 1){
                    if(this->row + 1 == this->para_list_size / 2)
                    {
                    } else {
                        this->table_view_arr_list[tab].tableView_right->selectRow(++this->row);
                        this->table_view_arr_list[tab].tableView_left->clearSelection();

                    }

                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_PLUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_MINUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            if(this->row != -1){
                this->index = this->row * 2 + this->column;
                action_set *as = new action_set(this->index);
                as->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = as;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:

            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~ActionConfigure();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;

    default:
        break;
    }


    return QWidget::event(event);
}
