#include "auxiliarymenu.h"
#include "ui_auxiliarymenu.h"

#include "./Resources/key_value.h"
#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QStringList>

#include <QTextCodec>
#include <QByteArray>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QScrollBar>

#include "./window/leftpane.h"
#include "./Resources/parameters.h"
#include "threads/comm_acs_thread.h"
#include "./Resources/error_code.h"
#include "dialog_usercheckandlogin.h"
//#include "./Resources/tcpclient.h"
#include <QDebug>

AuxiliaryMenu::AuxiliaryMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuxiliaryMenu)
{
    ui->setupUi(this);
    page_index = 0;
    this->setWindowFlags(windowFlags()|Qt::FramelessWindowHint);

    this->setGeometry(50, 10, 280, 240);

    QFile file(":/qss/auxiliarymenu.qss");
    if(!file.open(QFile::ReadOnly)){
        TLOG_INFO("open qss file failed in AuxiliaryMenu().");
    }else{
        QString styleSheet = file.readAll();
        this->setStyleSheet(styleSheet);
        file.close();
    }

    this->row = 0;
    this->widget_stack = WidgetStack::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->action_register = ActionRegister::getInstance();
    this->ss = SystemState::getInstance();

    connect(this, SIGNAL(change_user()), this->comm_acs_thread, SLOT(closeAllwindow()));

    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->setMargin(40);
    hlayout->setSpacing(0);

    QRect rec(0, 40, 480, 272-40);
    hlayout->setGeometry(rec);

    this->tableView = new QTableView();
    tableView->setShowGrid(false); //不显示格子线
    tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
    tableView->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    //tableView->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

    hlayout->addWidget(tableView);

    QStandardItemModel* model = new QStandardItemModel();
    model->setColumnCount(1);
    tableView->setModel(model);
    //设置宽度要在设置了model之后调用
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setVisible(false);
    //表中有被选中的行时，表头不变色
    tableView->horizontalHeader()->setHighlightSections(false);

    QFont font;

    this->index = 0;
    if(this->ss->get_system_state() == SystemState::Neighbour)
    {
        this->count = this->action_register->neighbour_action_aditional_list.size();
    } else if (this->ss->get_system_state() == SystemState::Group) {
        this->count = this->action_register->group_action_aditional_list.size();
    } else if (this->ss->get_system_state() == SystemState::Interval) {
        this->count = this->action_register->interval_action_aditional_list.size();
    }

    for(int i = 0; i < this->count; i++)
    {
        if(this->ss->get_system_state() == SystemState::Neighbour)
        {
            model->setItem(i, 0, new QStandardItem(this->action_register->neighbour_action_aditional_list[i].second.action_name));
        } else if (this->ss->get_system_state() == SystemState::Group) {
            model->setItem(i, 0, new QStandardItem(this->action_register->group_action_aditional_list[i].second.action_name));
        } else if (this->ss->get_system_state() == SystemState::Interval) {
            model->setItem(i, 0, new QStandardItem(this->action_register->interval_action_aditional_list[i].second.action_name));
        }
    }

    for(int row = 0; row < count; row++){
        font = model->item(row, 0)->font();
        font.setPointSize(14);
        model->item(row, 0)->setFont(font);
        model->item(row, 0)->setForeground(Qt::yellow);
        model->item(row, 0)->setTextAlignment(Qt::AlignHCenter);
    }
    this->tableView->selectRow(this->row);
}

AuxiliaryMenu::~AuxiliaryMenu()
{
    delete ui;
}

bool AuxiliaryMenu::event(QEvent *event)
{
    QString msg;
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);

    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_LEFT:
        {
            QString id;
            if(this->ss->get_system_state() == SystemState::Neighbour){
                if(this->action_register->neighbour_action_aditional_list.size() > this->row)
                    id = this->action_register->neighbour_action_aditional_list[this->row].first;
            } else if (this->ss->get_system_state() == SystemState::Group){
                if(this->action_register->group_action_aditional_list.size() > this->row)
                    id = this->action_register->group_action_aditional_list[this->row].first;
            } else if (this->ss->get_system_state() == SystemState::Interval){
                if(this->action_register->interval_action_aditional_list.size() > this->row)
                    id = this->action_register->interval_action_aditional_list[this->row].first;
            }
            if(!id.isEmpty())
            {
                msg = QString("config -auxiliaryID start ");
                msg += id;
            }
            this->comm_acs_thread->send_socket_msg_to_acs(msg);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
        }
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key())
        {
        case KeyValue::KEY_ARROW_RIGHT:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
        {
            //comm_acs_thread->control_timer->stop();
            QString id;
            if(this->ss->get_system_state() == SystemState::Neighbour){
                if(this->action_register->neighbour_action_aditional_list.size() > this->row)
                    id = this->action_register->neighbour_action_aditional_list[this->row].first;
            } else if (this->ss->get_system_state() == SystemState::Group){
                if(this->action_register->group_action_aditional_list.size() > this->row)
                    id = this->action_register->group_action_aditional_list[this->row].first;
            } else if (this->ss->get_system_state() == SystemState::Interval){
                if(this->action_register->interval_action_aditional_list.size() > this->row)
                    id = this->action_register->interval_action_aditional_list[this->row].first;
            }
            if(!id.isEmpty())
            {
                msg = QString("config -auxiliaryID stop ");
                msg += id;
            }
            this->comm_acs_thread->send_socket_msg_to_acs(msg);
            this->comm_acs_thread->cmdManager->lock_ui_off();

            break;
        }
        case KeyValue::KEY_ARROW_UP:
        {
            comm_acs_thread->control_timer->stop();

            if(this->row > 0){
                this->tableView->selectRow(--this->row);
            }

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_DOWN:
        {
            comm_acs_thread->control_timer->stop();

            if(this->row < (count - 1)){
                this->tableView->selectRow(++this->row);
            }

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_OK:
        {
            comm_acs_thread->control_timer->stop();

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~AuxiliaryMenu();

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
