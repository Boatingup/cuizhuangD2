#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>

#include "fault_display.h"
#include "ui_fault_display.h"
#include "./Resources/dbg_print.h"
#include "./Resources/command.h"
#include "generic_config.h"

extern QVector<QMap<QString, FAULT_INFO_S>> g_fault;
extern QMap<QString, FAULT_PARAM_S> g_fault_param;

fault_display::fault_display(QString& category, QWidget *parent):
    QWidget(parent),
    ui(new Ui::fault_display)
{

    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->category = category;
    this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    QFile file(":/qss/fault_display.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }
    this->setBackgroundRole(QPalette::Dark);
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
    tableView_left->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示水平滚动条
    tableView_left->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    tableView_left->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss
    tableView_left->setColumnWidth(0,400);
    //设置右半侧tableview
    tableView_right->setShowGrid(false); //不显示格子线
    tableView_right->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
    tableView_right->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示水平滚动条
    tableView_right->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    tableView_right->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss
    tableView_right->setColumnWidth(0,400);

    hlayout->addWidget(tableView_left);
    hlayout->addWidget(tableView_right);

    tab->setLayout(hlayout);

    tabWidget->addTab(tab, "");
    load_fault();

    QFont font = ui->title_label->font();
    font.setBold(true);
    ui->title_label->setFont(font); // 字体加粗
    ui->title_label->setText("故障诊断系统");
}

fault_display::~fault_display()
{
    delete ui;
}

void fault_display::fault_list_update()
{
    if(fault_list.isEmpty() == false)
        fault_list.clear();
    QPair<QString, FAULT_INFO_S> pair;
    pair.first = "no_fault";
    pair.second.fault_time = 0;
    pair.second.fault_display = "系统无故障";
    pair.second.fault_diagnosis = "未检测到故障";
    if(g_fault.isEmpty())
    {
        fault_list.append(pair);
        return;
    }
    int i;
    for(i=0; i<g_fault.size(); i++)
    {
        if(g_fault[i].isEmpty() == true)
            continue;

        for(QMap<QString, FAULT_INFO_S>::iterator itera = g_fault[i].begin(); itera != g_fault[i].end(); ++itera)
        {
            pair.first = itera.key();
            pair.second = itera.value();
            fault_list.append(pair);
        }
    }
    if(fault_list.isEmpty())
    {
        pair.first = "no_fault";
        pair.second.fault_time = 0;
        pair.second.fault_display = "系统无故障";
        pair.second.fault_diagnosis = "未检测到故障";
        fault_list.append(pair);
    }
    return ;
}

void fault_display::load_fault()
{
        int tab = 0;
        fault_list_update();
        struct tableViewArr arr = this->table_view_arr_list.at(tab);
        this->fault_list_size = fault_list.size();
        QStandardItemModel* model = nullptr;
        QStandardItemModel* model_r = nullptr;
        if(arr.tableView_left->model()){

            model = (QStandardItemModel*)arr.tableView_left->model();
            model->removeRows(0, model->rowCount());
        }else {
            model = new QStandardItemModel();
            model->setColumnCount(2);

            arr.tableView_left->setModel(model);
            arr.tableView_left->setColumnWidth(0, 400);
            arr.tableView_left->verticalHeader()->setDefaultSectionSize(40);
            arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            arr.tableView_left->horizontalHeader()->setVisible(false);
        }

        if(arr.tableView_right->model()){
            model_r = static_cast<QStandardItemModel*>(arr.tableView_right->model());
            model_r->removeRows(0, model_r->rowCount());
        } else {
            model_r = new QStandardItemModel();
            model_r->setColumnCount(2);
            arr.tableView_right->setModel(model_r);
            arr.tableView_right->setColumnWidth(0, 400);
            arr.tableView_right->verticalHeader()->setDefaultSectionSize(40);
            arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            arr.tableView_right->horizontalHeader()->setVisible(false);

        }
        QFont font;
        QList <QPair<QString,FAULT_INFO_S>>::const_iterator it = fault_list.constBegin();
        int i = 0;
        int row = 0;
        if(fault_list.size() == 0)
        {
            TLOG_INFO("action.xml is null");
            return;
        }
        while (it != fault_list.constEnd()) {
            model->setItem(row, 0, new QStandardItem(it->second.fault_display)); //设置tablebiew中的文字内容
            model->item(row, 0)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
            font = model->item(row, 0)->font();
            font.setFamily("YouSheBiaoTiHei");
            font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加
            model->item(row, 0)->setFont(font);
            i+=2;
            if(i > fault_list.size() - 1)
            {
                break;
            }
            row++;
            it += 2;
        }
        it = fault_list.constBegin();
        it++;
        i = 1;
        row = 0;
        while (it != fault_list.constEnd())
        {
            model_r->setItem(row, 0, new QStandardItem(it->second.fault_display)); //设置tablebiew中的文字内容
            model_r->item(row, 0)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
            font = model_r->item(row, 0)->font();
            font.setFamily("YouSheBiaoTiHei");
            font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加
            model_r->item(row, 0)->setFont(font);
            i+=2;
            if(i > fault_list.size() - 1)
            {
                break;
            }
            row++;
            it += 2;
        }
        if(fault_list.size() % 2 == 1)
        {
            int last_row = fault_list.size() / 2;
            model_r->setItem(last_row, 0, new QStandardItem(" ")); //设置tablebiew中的文字内容

            model_r->item(last_row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示

            font = model_r->item(last_row, 0)->font();

            font.setFamily("YouSheBiaoTiHei");
            font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

            model_r->item(last_row, 0)->setFont(font);
        }
        arr.tableView_left->clearSelection();
        arr.tableView_right->clearSelection();
}

bool fault_display::event(QEvent *event)
{
    QString strtab;

    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            this->column = 1;
            if((fault_list_size %2) == 1)
            {
                if(this->row == (fault_list_size/2))
                {
                     this->column = 0;
                }
                else {
                      this->table_view_arr_list[tab].tableView_left->clearSelection();
                    this->table_view_arr_list[tab].tableView_right->selectRow(this->row);
                }
            }
            else {
                this->table_view_arr_list[tab].tableView_left->clearSelection();
                this->table_view_arr_list[tab].tableView_right->selectRow(this->row);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_LEFT:
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
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
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            if(this->column == 0){ //in the left table view
                if(this->row < ((this->fault_list_size % 2) ? (this->fault_list_size + 1) / 2 : (this->fault_list_size / 2)) - 1){
                    this->table_view_arr_list[tab].tableView_left->selectRow(++this->row);
                    this->table_view_arr_list[tab].tableView_right->clearSelection();
                }
            }else{ //in the right table view
                if(this->row < ((this->fault_list_size % 2) ? (this->fault_list_size + 1) / 2 : (this->fault_list_size / 2)) - 1){
                    if(this->row + 1 == this->fault_list_size / 2)
                    {
                         //int k = 0;
                    }
                    else {
                        this->table_view_arr_list[tab].tableView_right->selectRow(++this->row);
                        this->table_view_arr_list[tab].tableView_left->clearSelection();
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_PLUS:
        {
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            this->row = -1;
            this->column = 0;
            load_fault();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_MINUS:
        {
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            generic_config *dia = new generic_config(TYPE_GENERIC_CONNECT_BOARD);
            dia->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = dia;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F1:
        {
            comm_acs_thread->control_timer->stop();
            generic_config *dia = new generic_config(TYPE_GENERIC_SENSOR_ANGLE);
            dia->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = dia;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F2:
        {
            comm_acs_thread->control_timer->stop();
            generic_config *dia = new generic_config(TYPE_GENERIC_SENSOR_HIGH);
            dia->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = dia;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F3:
        {
            comm_acs_thread->control_timer->stop();
            generic_config *dia = new generic_config(TYPE_GENERIC_DRIVER_VOLT);
            dia->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = dia;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F4:
        {
            comm_acs_thread->control_timer->stop();
            generic_config *dia = new generic_config(TYPE_GENERIC_DRIVER_CUR);
            dia->show();
            this->widget_stack->push(this);
            this->comm_acs_thread->cmdManager->window = dia;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            if(this->row != -1){
                this->index = this->row * 2 + this->column;
                fault_diagnosis_diaplsy *as = new fault_diagnosis_diaplsy(fault_list.at(this->index).second.fault_display+"诊断", \
                                          fault_list.at(this->index).second.fault_diagnosis, fault_list.at(this->index).second.index);
                as->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = as;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~fault_display();
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

