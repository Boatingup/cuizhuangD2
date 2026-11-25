#include "leftpane.h"
#include "ui_leftpane.h"

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

#include "../Resources/key_value.h"
#include "../Resources/auto_follow_config.h"
#include "../threads/para_synchronize_thread.h"
#include "parameter_config.h"
#include "menupage.h"
#include "../Resources/error_code.h"
#include <mutex>

extern QString Category_Index;
extern std::mutex para_mutex_thread;
extern bool is_large;
extern QString syzk_user;
extern QList<sensor_info> sensor_list;
extern QList<sensor_info> ad_list;
extern QList<sensor_info> wireless_list;

extern QWidget *pMainPage;

extern bool key_right_healthy_flag;
extern QList<info_prara> readonly_tabs;
bool key_right_healthy_flag = false;
QList<info_prara> readonly_tabs;
int leftPane::FindModelIndex(int index,int &index_num)
{
     for(int i = 0; i < model_list.size();i++)
     {
         if(model_list[i].index == index)
         {
             index_num = i;
             return 1;
         }
     }
     return 0;
}

void leftPane::set_array_tab(struct tableViewArr arr, QStandardItemModel *model)
{
    arr.tableView_left->setModel(model);
    //设置宽度要在设置了model之后调用
    arr.tableView_left->setColumnWidth(0, 240);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_left->horizontalHeader()->setVisible(false);

    arr.tableView_left->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_left->horizontalHeader()->setHighlightSections(false);

    QFont font;

    for(int i=0, row = 0; i < para_list[0].size(); i+=2, row++){//左半侧存放偶数（从0开头）项参数
        QStringList strlist = para_list[0][i].split(":");

        model->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model->setItem(row, 1, new QStandardItem(strlist[1]));

        model->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);
    }

    QStandardItemModel* model_r = new QStandardItemModel();
    model_r->setColumnCount(2);
    model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_right->setModel(model_r);
    //设置宽度要在设置了model之后调用

    arr.tableView_right->setColumnWidth(0, 240);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_right->horizontalHeader()->setVisible(false);

    arr.tableView_right->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_right->horizontalHeader()->setHighlightSections(false);


    for(int i=1, row = 0; i<para_list[0].size(); i+=2, row++){//左半侧存放偶数（从1开头）项参数
        QStringList strlist = para_list[0][i].split(":");

        model_r->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model_r->setItem(row, 1, new QStandardItem(strlist[1]));

        model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model_r->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model_r->item(row, 0)->setFont(font);
        model_r->item(row, 1)->setFont(font);
    }
}

void leftPane::timer_cb()
{
    int index_num = -1;
    if(category != "healthy")
        return ;

    QFont font;
    font = model_list[0].model_left->item(0,0)->font();
    font.setFamily("YouSheBiaoTiHei");
    font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

    for(int i = 0; i< model_list.size(); i++)
    {
        if(this->comm_acs_thread->cmdManager->findIndexByName("WirelessData",index_num) == 1)
        {
            int num = -1;
            if(FindModelIndex(index_num,num) != 1)
                continue;

            for(int j = 0;j < model_list[num].total; j++)
            {
                if(j % 2 == 0){
                    model_list[num].model_left->item(j/2, 1)->setFont(font);
                    model_list[num].model_left->setItem(j/2, 1, new QStandardItem((wireless_list[j].value)));

                } else {
                    model_list[num].model_right->item(j/2, 1)->setFont(font);
                    model_list[num].model_right->setItem(j/2, 1, new QStandardItem((wireless_list[j].value)));
                }
            }

        } else if (this->comm_acs_thread->cmdManager->findIndexByName("SenSorData",index_num) == 1){
            int num = -1;
            if(FindModelIndex(index_num,num) != 1)
                continue;

            for(int j = 0;j < model_list[num].total; j++)
            {
                if(j % 2 == 0){
                    model_list[num].model_left->item(j/2, 1)->setFont(font);
                    if(sensor_list[j].name.contains("Inclinationsensor")){
                        model_list[num].model_left->setItem(j/2, 1, new QStandardItem(sensor_list[j].value));
                    } else if (sensor_list[j].name.contains("ConnectTotal")) {
                        model_list[num].model_left->setItem(j/2, 1, new QStandardItem(QString::number(sensor_list[j].value.toDouble(), 'f', 1)));
                    } else {
                        model_list[num].model_left->setItem(j/2, 1, new QStandardItem((sensor_list[j].value)));
                    }
                    //set_array_tab(model_list[num].tab_array, model_list[num].model_left);
                } else {
                    model_list[num].model_right->item(j/2, 1)->setFont(font);
                    if(sensor_list[j].name.contains("Inclinationsensor")){
                        model_list[num].model_right->setItem(j/2, 1, new QStandardItem((sensor_list[j].value)));
                    } else if (sensor_list[j].name.contains("ConnectTotal")) {
                        model_list[num].model_right->setItem(j/2, 1, new QStandardItem(QString::number(sensor_list[j].value.toDouble(), 'f', 1)));
                    } else {
                        model_list[num].model_right->setItem(j/2, 1, new QStandardItem((sensor_list[j].value)));
                    }
                }
            }
        }
    }
#if 0
    table_view_arr_list[tab].tableView_left->setColumnWidth(0, 240);
    table_view_arr_list[tab].tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_view_arr_list[tab].tableView_left->horizontalHeader()->setVisible(false);
    table_view_arr_list[tab].tableView_left->verticalHeader()->setDefaultSectionSize(40);
    table_view_arr_list[tab].tableView_left->horizontalHeader()->setHighlightSections(false);

    table_view_arr_list[tab].tableView_right->setColumnWidth(0, 240);
    table_view_arr_list[tab].tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_view_arr_list[tab].tableView_right->horizontalHeader()->setVisible(false);
    table_view_arr_list[tab].tableView_right->verticalHeader()->setDefaultSectionSize(40);
    table_view_arr_list[tab].tableView_right->horizontalHeader()->setHighlightSections(false);
#endif
    if(this->column == 0){
        table_view_arr_list[tab].tableView_left->selectRow(this->row);
    } else {
        table_view_arr_list[tab].tableView_right->selectRow(this->row);
    }
}

leftPane::leftPane(QString& category, QWidget *parent)
      : QWidget(parent)
      , ui(new Ui::leftPane)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(800, 480);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();

    this->category = category;
    this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    connect(this, SIGNAL(signalloadframe(int)), this, SLOT(slotloadframe(int)), Qt::QueuedConnection);

    QFile file(":/qss/leftpane.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }

    this->row = -1;  //光标所在行,-1是为了按向下按键时+1后为0，对应list中下标为0
    this->column = 0;//光标所在列， 0为左列，1为右列
    this->index  = 0;//在subblock标签下中对应参数的索引
    this->tab = 0; //subblock的对应标签

    int size = this->para->getBlockNumber(category);
    qDebug() << "size = " << size << "leftpane category "  << category;
    this->tabWidget = new QTabWidget(this);
    tabWidget->move(0, 40);
    tabWidget->setGeometry(0, 40, 800, 440);

#if 0
    QTableView *tableView = new QTableView;
    CustomHeaderView *headerView = new CustomHeaderView(Qt::Horizontal, tableView);
    tableView->setHorizontalHeader(headerView);
#endif
    QTableView* tableView_left;
    QTableView* tableView_right;

    for(int i = 0; i < 1; i++){
        QWidget *tab = new QWidget();
        QHBoxLayout* hlayout = new QHBoxLayout();
        hlayout->setMargin(0);
        hlayout->setSpacing(0);

        struct tableViewArr table_view_arr;
        tableView_left = table_view_arr.tableView_left = new QTableView();
        tableView_right = table_view_arr.tableView_right = new QTableView();
#if 0
        CustomHeaderView *headerView = new CustomHeaderView(Qt::Horizontal, tableView_left);
        tableView_left->setHorizontalHeader(headerView);
#endif
        table_view_arr_list.append(table_view_arr);

        //设置左半侧tableview
        tableView_left->setShowGrid(false); //不显示格子线
        tableView_left->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_left->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示水平滚动条
        tableView_left->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
        tableView_left->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        //设置右半侧tableview
        tableView_right->setShowGrid(false); //不显示格子线
        tableView_right->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_right->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tableView_right->verticalHeader()->setVisible(false); //垂头（也就是行号）//不显示水平滚动条
        tableView_right->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        hlayout->addWidget(tableView_left);
        hlayout->addWidget(tableView_right);

        tab->setLayout(hlayout);

        QString tabname = this->para->tab_name.at(this->para->category_index.value(category)).at(i);
        tabWidget->addTab(tab, tabname);

        load_parameters(0);
    }

    QFont font = ui->label_category->font();
    font.setBold(true);
    ui->label_category->setFont(font); // 字体加粗
    ui->label_category->setText(this->para->category_name.value(category));

    //update the data  start  ad
    this->timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_cb()));
    this->timer->start(500);
    timer_cb();

    if (category == "follow_para") {
        connect(ParaSynchronizeThread::getInstance(),
                &ParaSynchronizeThread::sig_update_follow_stages,
                this, &leftPane::on_follow_stages_updated);
    }

    emit signalloadframe(size);
}

leftPane::~leftPane()
{
    delete ui;
}

void leftPane::slotloadframe(int size)
{
    TLOG_INFO("#################slotloadframe##############category:%s ,size:%d\n", category.toUtf8().data(), size);
    QTableView* tableView_left;
    QTableView* tableView_right;

    for(int i = 1; i < size; i++){
        QWidget *tab = new QWidget();
        QHBoxLayout* hlayout = new QHBoxLayout();
        hlayout->setMargin(0);
        hlayout->setSpacing(0);

        struct tableViewArr table_view_arr;
        tableView_left = table_view_arr.tableView_left = new QTableView();
        tableView_right = table_view_arr.tableView_right = new QTableView();

        this->table_view_arr_list.append(table_view_arr);

        //设置左半侧tableview
        tableView_left->setShowGrid(false); //不显示格子线
        tableView_left->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_left->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示水平滚动条
        tableView_left->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
        tableView_left->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        //设置右半侧tableview
        tableView_right->setShowGrid(false); //不显示格子线
        tableView_right->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
        tableView_right->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tableView_right->verticalHeader()->setVisible(false); //垂头（也就是行号）//不显示水平滚动条
        tableView_right->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

        hlayout->addWidget(tableView_left);
        hlayout->addWidget(tableView_right);

        tab->setLayout(hlayout);
        QString tabname = this->para->tab_name.at(this->para->category_index.value(category)).at(i);

        this->tabWidget->addTab(tab, tabname);
    }
}

bool leftPane::is_stage_display_tab(int tab) const
{
    if (category != "follow_para")
        return false;
    QString cat = category;
    QString tabName = this->para->query_tab_name_by_index(cat, tab);
    return tabName == "stage_display";
}

void leftPane::on_follow_stages_updated()
{
    if (category != "follow_para")
        return;
    // Prefer in-memory cache already updated by saveFollowConfig;
    // reload file only if cache empty (defensive).
    if (!AutoFollowConfig::getInstance()->hasConfig())
        AutoFollowConfig::getInstance()->loadFromFile();
    if (is_stage_display_tab(this->tab)) {
        load_stage_display(this->tab);
    }
}

void leftPane::load_stage_display(int tab)
{
    if (tab < 0 || tab >= table_view_arr_list.size())
        return;

    // Use cache if present; otherwise load last saved file.
    if (!AutoFollowConfig::getInstance()->hasConfig())
        AutoFollowConfig::getInstance()->loadFromFile();
    QStringList rows = AutoFollowConfig::getInstance()->buildDisplayRows();

    this->para_list.clear();
    this->para_list.append(rows);
    this->para_list_size = this->para->getBlockNumber(category);
    list_index.clear();
    QList<int> idx;
    for (int i = 0; i < rows.size(); ++i)
        idx.append(i);
    list_index.append(idx);

    // mark as readonly so OK cannot edit
    readonly_tabs.clear();
    info_prara ro;
    ro.tab = tab;
    ro.name = "stage_display";
    readonly_tabs.append(ro);

    struct tableViewArr arr = table_view_arr_list.at(tab);

    QStandardItemModel* model = new QStandardItemModel();
    model->setColumnCount(2);
    model->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model->setHeaderData(1, Qt::Horizontal, QString("数值"));

    // 仅作用于阶段显示页的左右表，不影响跟机状态/基本参数/动作参数及其它模块
    auto setup_stage_table = [](QTableView *view) {
        view->horizontalHeader()->setVisible(false);
        view->horizontalHeader()->setHighlightSections(false);
        view->verticalHeader()->setDefaultSectionSize(40);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setTextElideMode(Qt::ElideNone);
        view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    };

    arr.tableView_left->setModel(model);
    setup_stage_table(arr.tableView_left);

    QFont font;
    font.setFamily("YouSheBiaoTiHei");
    font.setPointSize(24);

    for (int i = 0, row = 0; i < rows.size(); i += 2, row++) {
        QStringList strlist = rows[i].split(":");
        QString name = strlist.value(0);
        QString value = strlist.size() > 1 ? strlist.mid(1).join(":") : "";
        model->setItem(row, 0, new QStandardItem(name));
        model->setItem(row, 1, new QStandardItem(value));
        model->item(row, 0)->setTextAlignment(Qt::AlignCenter);
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);
    }

    QStandardItemModel* model_r = new QStandardItemModel();
    model_r->setColumnCount(2);
    model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_right->setModel(model_r);
    setup_stage_table(arr.tableView_right);

    for (int i = 1, row = 0; i < rows.size(); i += 2, row++) {
        QStringList strlist = rows[i].split(":");
        QString name = strlist.value(0);
        QString value = strlist.size() > 1 ? strlist.mid(1).join(":") : "";
        model_r->setItem(row, 0, new QStandardItem(name));
        model_r->setItem(row, 1, new QStandardItem(value));
        model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter);
        model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        model_r->item(row, 0)->setFont(font);
        model_r->item(row, 1)->setFont(font);
    }

    arr.tableView_left->clearSelection();
    arr.tableView_right->clearSelection();
}

void leftPane::load_parameters(int tab,int row,bool flag)
{
    TLOG_INFO("in load_parameters(), tab is %d", tab);
    if(flag == 1)
    {
        this->row = -1;
        this->column = 0;
    }
    if (is_stage_display_tab(tab)) {
        load_stage_display(tab);
        return;
    }
    list_index.clear();
    int size = 0;
    //this->para_list = this->para->read(category,readonly_tabs,list_index);
    this->para_list = this->para->read(category,readonly_tabs,tab,list_index,size);

    struct tableViewArr arr = table_view_arr_list.at(tab);
    this->para_list_size = size;

    struct model_index p1;
    QStandardItemModel* model = new QStandardItemModel();
    p1.model_left = model;
    p1.index = tab;
    int num_index = -1;

    //model_list.push_back(p1);
    model->setColumnCount(2);
    model->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_left->setModel(model);
    //设置宽度要在设置了model之后调用
    arr.tableView_left->setTextElideMode(Qt::ElideRight);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_left->setColumnWidth(0, 240);
    arr.tableView_left->horizontalHeader()->setVisible(false);
    arr.tableView_left->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_left->horizontalHeader()->setHighlightSections(false);

    QFont font;
    font.setFamily("YouSheBiaoTiHei");
    for(int i=0, row = 0; i < para_list[0].size(); i+=2, row++){//左半侧存放偶数（从0开头）项参数
        QStringList strlist = para_list[0][i].split(":");
        if(category == "control_para" && tab == 5 && strlist[1].toStdString().find("-") != string::npos)
        {
            QStringList slist = strlist[1].split("-");
            strlist[1] = "";
            strlist[1] = slist[0] + ":" + slist[1];
        }
        model->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model->setItem(row, 1, new QStandardItem(strlist[1]));

        model->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);
    }

    QStandardItemModel* model_r = new QStandardItemModel();
    model_r->setColumnCount(2);
    model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));

    p1.total =  para_list[0].size();
    p1.model_right = model_r;

    if(category == "healthy")
    {
        if(FindModelIndex(tab,num_index) == 1)
        {
            model_list.removeAt(num_index);
            model_list.push_back(p1);
        }
        else
        {
            model_list.push_back(p1);
        }
    }

    arr.tableView_right->setModel(model_r);

    arr.tableView_right->setTextElideMode(Qt::ElideRight);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_right->setColumnWidth(0, 240);
    arr.tableView_right->horizontalHeader()->setVisible(false);
    arr.tableView_right->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_right->horizontalHeader()->setHighlightSections(false);

    for(int i=1, row = 0; i<para_list[0].size(); i+=2, row++){//左半侧存放偶数（从1开头）项参数
        QStringList strlist = para_list[0][i].split(":");

        model_r->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model_r->setItem(row, 1, new QStandardItem(strlist[1]));

        model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model_r->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model_r->item(row, 0)->setFont(font);
        model_r->item(row, 1)->setFont(font);
    }

    arr.tableView_left->clearSelection();
    arr.tableView_right->clearSelection();


    if(flag == 0)
    {
        if(row % 2 == 0)
        {
            arr.tableView_left->selectRow(row / 2);
        }

        if(row %2 != 0)
        {
            arr.tableView_right->selectRow(row / 2);
        }
    }
    readonly_tabs.clear();

    if(tab == 0 && row == 0)//修改主控时间则重新计时
    {
        QString category_name = "system";
        int tab2 = 0;
        QString parameter_name = "controlTime";
        QString label ="real_value";

        QString str_main_control_time = this->para->query(category_name, tab2, parameter_name, label);
        if(str_main_control_time != ""){
            this->comm_acs_thread->main_control_time = 1000 * str_main_control_time.toFloat();
        }else{
            this->comm_acs_thread->main_control_time = 10000; //10s as default time back to main page.
        }

        this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

        parameter_name = "controlAlertTime";
        str_main_control_time = this->para->query(category_name, tab2, parameter_name, label);
        this->comm_acs_thread->m_alter_control_time = this->comm_acs_thread->main_control_time;
        if(str_main_control_time != ""){
            this->comm_acs_thread->m_alter_control_time = 1000 * str_main_control_time.toFloat();
        }

        parameter_name = "groupAlertTime";
        str_main_control_time = this->para->query(category_name, tab2, parameter_name, label);
        this->comm_acs_thread->m_group_control_time = this->comm_acs_thread->main_control_time;
        if(str_main_control_time != ""){
            this->comm_acs_thread->m_group_control_time = 1000 * str_main_control_time.toFloat();
        }

    }
}

void leftPane::load_parameters(int tab)
{
    TLOG_INFO("tab is over in load_parameters(), tab is %d", tab);
    if (is_stage_display_tab(tab)) {
        load_stage_display(tab);
        return;
    }
    readonly_tabs.clear();
    list_index.clear();
    int size = 0;
    this->para_list = this->para->read(category,readonly_tabs,tab,list_index,size);
    struct tableViewArr arr = table_view_arr_list.at(tab);
    this->para_list_size = size;

    struct model_index p1;
    QStandardItemModel* model = new QStandardItemModel();
    int num_index = -1;
    model->setColumnCount(2);
    model->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_left->setModel(model);
    //设置宽度要在设置了model之后调用
    arr.tableView_left->setTextElideMode(Qt::ElideRight);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_left->setColumnWidth(0, 240);
    arr.tableView_left->horizontalHeader()->setVisible(false);

    arr.tableView_left->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_left->horizontalHeader()->setHighlightSections(false);

    QFont font;

    for(int i=0, row = 0; i < para_list[0].size(); i+=2, row++){//左半侧存放偶数（从0开头）项参数
        QStringList strlist = para_list[0][i].split(":");
        if(category == "control_para" && tab == 5 && strlist[1].toStdString().find("-") != string::npos)
        {
            QStringList slist = strlist[1].split("-");
            strlist[1] = "";
            strlist[1] = slist[0] + ":" + slist[1];
        }
        model->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model->setItem(row, 1, new QStandardItem(strlist[1]));

        model->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);
    }

    QStandardItemModel* model_r = new QStandardItemModel();
    model_r->setColumnCount(2);
    model_r->setHeaderData(0, Qt::Horizontal, QString("名称"));
    model_r->setHeaderData(1, Qt::Horizontal, QString("数值"));

    arr.tableView_right->setModel(model_r);
    //设置宽度要在设置了model之后调用

    arr.tableView_right->setTextElideMode(Qt::ElideRight);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    arr.tableView_right->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    arr.tableView_right->setColumnWidth(0, 240);
    arr.tableView_right->horizontalHeader()->setVisible(false);

    arr.tableView_right->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    arr.tableView_right->horizontalHeader()->setHighlightSections(false);


    for(int i=1, row = 0; i<para_list[0].size(); i+=2, row++){//左半侧存放偶数（从1开头）项参数
        QStringList strlist = para_list[0][i].split(":");

        model_r->setItem(row, 0, new QStandardItem(strlist[0])); //设置tablebiew中的文字内容
        model_r->setItem(row, 1, new QStandardItem(strlist[1]));

        model_r->item(row, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
        model_r->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        font = model_r->item(row, 0)->font();
        font.setFamily("YouSheBiaoTiHei");
        font.setPointSize(24);   //设置tableview中文字的大小，如需设置其他可继续添加

        model_r->item(row, 0)->setFont(font);
        model_r->item(row, 1)->setFont(font);
    }

    p1.index = tab;
    p1.total =  para_list[0].size();

    p1.model_left = model;
    p1.model_right = model_r;
    p1.tab_array = table_view_arr_list.at(tab);
    if(category == "healthy")
    {
        if(FindModelIndex(tab,num_index) == 1)
        {
            model_list.removeAt(num_index);
            model_list.push_back(p1);
        } else {
            model_list.push_back(p1);
        }
    }

    arr.tableView_left->clearSelection();
    arr.tableView_right->clearSelection();
    TLOG_INFO("tab is over");
}


int  leftPane::FindNameByIndex(QString & name,int &index)
{
     for(int i=0 ; i <readonly_tabs.size();i++)
     {
        if(readonly_tabs[i].name == name)
        {
            index = i;
            return 1;
        }
     }
     return 0;
}


bool leftPane::InRoList()
{
    for(QList <info_prara>::const_iterator it = readonly_tabs.begin(); it != readonly_tabs.end(); ++it){
        if((it->tab) == tab)
        return true;
    }
    return false;

}

bool leftPane::event(QEvent *event)
{
    ParameterConfig* para_config;
    QString strtab;
    int isReadonly = -1;
    if(InRoList()==true)
    {
        isReadonly = 1;
    }
    else {
        isReadonly = 0;
    }

    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);

    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->column = 1;
            if((para_list[0].size() %2) == 1)
            {
                if(this->row == (para_list[0].size()/2))
                {
                     this->column = 0;
                }
                else {
                    table_view_arr_list[tab].tableView_left->clearSelection();
                    table_view_arr_list[tab].tableView_right->selectRow(this->row);
                }
            }
            else {
                table_view_arr_list[tab].tableView_left->clearSelection();
                table_view_arr_list[tab].tableView_right->selectRow(this->row);

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_LEFT:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->column = 0;
            table_view_arr_list[tab].tableView_right->clearSelection();
            table_view_arr_list[tab].tableView_left->selectRow(this->row);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_UP:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->column == 0){ //in the left table view
                if(this->row > 0){
                    table_view_arr_list[tab].tableView_left->selectRow(--this->row);
                    table_view_arr_list[tab].tableView_right->clearSelection();
                }
            }else{  // in the right table view
                if(this->row > 0){
                    table_view_arr_list[tab].tableView_right->selectRow(--this->row);
                    table_view_arr_list[tab].tableView_left->clearSelection();
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->column == 0){ //in the left table view
                if(this->row < ((this->para_list[0].size() % 2) ? (this->para_list[0].size() + 1) / 2 : (this->para_list[0].size() / 2)) - 1){
                    table_view_arr_list[tab].tableView_left->selectRow(++this->row);
                    table_view_arr_list[tab].tableView_right->clearSelection();
                }
            }else{ //in the right table view
                if(this->row < ((this->para_list[0].size() % 2) ? (this->para_list[0].size() + 1) / 2 : (this->para_list[0].size() / 2)) - 1){
                    if(this->row + 1 == this->para_list[0].size() / 2)
                    {
                         //int k = 0;
                    }
                    else {
                        table_view_arr_list[tab].tableView_right->selectRow(++this->row);
                        table_view_arr_list[tab].tableView_left->clearSelection();
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_PLUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(tab > 0){
                this->tabWidget->setCurrentIndex(--tab);
                table_view_arr_list[tab].tableView_left->clearSelection();
                table_view_arr_list[tab].tableView_right->clearSelection();
                this->row = -1;
                this->column = 0;
                this->index = 0;
            }
            load_parameters(tab);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_MINUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(tab < this->para_list_size - 1){
                load_parameters(++tab);
                this->tabWidget->setCurrentIndex(tab);
                table_view_arr_list[tab].tableView_left->clearSelection();
                table_view_arr_list[tab].tableView_right->clearSelection();

                this->row = -1;
                this->column = 0;
                this->index = 0;
            }
            /*
            if(tab == 2)
            {
                total_load = 1;
            }*/
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            if(isReadonly == 1)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                return QWidget::event(event);
            }
            comm_acs_thread->control_timer->stop();
            if(this->row != -1){
                strtab = this->category;
                this->index = this->row * 2 + this->column;//界面上的index
                is_large = false;
                QString write_access = "write";
                if(this->para->getAccess(strtab,tab,list_index[0][index],write_access) == 1)//当有权限时打开
                {
                    TLOG_INFO("%s %d %d %d\n", strtab.toUtf8().data(), tab, index, list_index[0][index]);
                    cout << strtab.toUtf8().data() <<" tab "<< tab << " " << index << " list_index " << list_index[0][index] << endl;
                    para_config = new ParameterConfig(strtab, tab, index, list_index[0][index]);
                    para_config->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = para_config;
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
        {
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            if(key_right_healthy_flag == true)
            {
                QString msg = QString("config -m 1");
                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                key_right_healthy_flag = false;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
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









