#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMap>
#include <QString>
#include "driver_chn_config.h"
#include "ui_driver_chn_config.h"
#include "./Resources/dbg_print.h"
#include "syncparafile.h"
#include "driver_chn_set.h"
#include <unistd.h>
#include <sys/reboot.h>

QMap<QString,int> g_driver_match;
QMap<QString,ACTION_MATCH_S> g_action_match;

QMap<QString,DRIVER_CHN_S> g_action_driver_match;
QList<QPair<QString,QString>> g_key_action_match;
QMap<QString,QString> g_key_value_match;

driver_chn_config::driver_chn_config(QString& category, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::driver_chn_config)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->driver_sync = driver_param_remote_sync::getInstance();
    this->category = category;
    //this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->comm_acs_thread->control_timer->stop();
    QFile file(":/qss/driver_chn_config.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }
    this->setBackgroundRole(QPalette::Dark);

    this->row = -1;  //光标所在行,-1是为了按向下按键时+1后为0，对应list中下标为0
    this->index  = 0;//在subblock标签下中对应参数的索引
    this->tab = 0; //subblock的对应标签
    this->para = Parameters::getInstance();
    this->tabWidget = new QTabWidget(this);
    update_param_flag = false;
    tabWidget->move(0, 40);
    tabWidget->setGeometry(0, 40, 480, 232);

    QTableView* tableView_left;

    TLOG_INFO("para_list.size() : %d", para_list.size());

    QWidget *tab = new QWidget();
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
    hlayout->setSpacing(0);

    struct tableViewArr table_view_arr;
    tableView_left = table_view_arr.tableView_left = new QTableView();

    table_view_arr_list.append(table_view_arr);

    //设置左半侧tableview
    tableView_left->setShowGrid(false); //不显示格子线
    tableView_left->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
    tableView_left->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    tableView_left->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss


    hlayout->addWidget(tableView_left);

    tab->setLayout(hlayout);

    tabWidget->addTab(tab, "驱动器通道配置");
    tabWidget->setStyleSheet("QTabBar::tab { height: 20px; width: 150px; }");

    load_config();

    QFont font = ui->title_label->font();
    font.setBold(true);
    ui->title_label->setFont(font); // 字体加粗
    ui->title_label->setText(this->para->category_name.value(category));
}

driver_chn_config::~driver_chn_config()
{
    delete m_ItemSelectionModel;
    delete ui;
}

bool driver_chn_config::load_xml(QString& file_path){
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("hmi:open fault xml file failed.");
        return false;
    }
    if(!this->driver_xml.setContent(&file))
    {
        file.close();
        TLOG_ERROR("hmi: Dom set fault xml content failed.");
        return false;
    }
    file.close();

    QDomElement root = this->driver_xml.documentElement(); //根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    if(node.isNull())
        return false;

    QDomNode match_node = node.firstChild(); //获得第一个子节点

    if(match_node.isNull())
        return false;

    int display_num = match_node.toElement().text().toInt();
    //TLOG_INFO("%d\n", display_num);
    match_node = match_node.nextSibling();

    int cur_no = 0;
    ACTION_DRIVER_KEY_INFO_S action_param;
    QPair<QString, QString> pair;
    DRIVER_CHN_S driver_chn;
    QString key_name;
    QString action_name;
    while(!match_node.isNull())
    {
        if(++cur_no > display_num)
            break ;
        QDomNode display_node = match_node.firstChild();
        key_name = display_node.toElement().text();
        display_node = display_node.nextSibling();
        action_name = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver1[0] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver1[1] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver1[2] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver1[3] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver2[0] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver2[1] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver2[2] = display_node.toElement().text();
        display_node = display_node.nextSibling();
        driver_chn.driver2[3] = display_node.toElement().text();

        pair.first = key_name;
        pair.second = action_name;
        g_key_action_match.append(pair);
        g_action_driver_match.insert(action_name, driver_chn);
        match_node = match_node.nextSibling();
    }
    cur_no = 0;
    node = node.nextSibling();
    match_node = node.firstChild();
    while(!match_node.isNull())
    {
        g_driver_match.insert(match_node.toElement().text(), ++cur_no);
        match_node = match_node.nextSibling();
    }

    node = node.nextSibling();//key
    match_node = node.firstChild();
    QString _key_name;
    QString _key_value;
    while(!match_node.isNull())
    {
        QDomNode display_node = match_node.firstChild();
        _key_name = display_node.toElement().text();
        display_node = display_node.nextSibling();
        _key_value = display_node.toElement().text();
        g_key_value_match.insert(_key_name, _key_value);
        match_node = match_node.nextSibling();
    }

    node = node.nextSibling();//action
    match_node = node.firstChild();
    driver_chn.driver1[0] = "NONE";
    driver_chn.driver1[1] = "NONE";
    driver_chn.driver1[2] = "NONE";
    driver_chn.driver1[3] = "NONE";
    driver_chn.driver2[0] = "NONE";
    driver_chn.driver2[1] = "NONE";
    driver_chn.driver2[2] = "NONE";
    driver_chn.driver2[3] = "NONE";
    ACTION_MATCH_S action;
    while(!match_node.isNull())
    {
        QDomNode display_node = match_node.firstChild();
        action_name = display_node.toElement().text();
        display_node = display_node.nextSibling();
        action.action_no = display_node.toElement().text().toInt();
        action.occurpied = false;
        g_action_match.insert(action_name, action);
        if(g_action_driver_match.find(action_name) == g_action_driver_match.end()){
            g_action_driver_match.insert(action_name, driver_chn);
        }
        match_node = match_node.nextSibling();
    }
    return true;
}

void driver_chn_config::update_driver_chn_config_xml()
{
    QString file_path=USED_XML_PATH;
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("hmi:open fault xml file failed.");
        return ;
    }
    if(!this->driver_xml.setContent(&file))
    {
        file.close();
        TLOG_ERROR("hmi: Dom set fault xml content failed.");
        return ;
    }
    file.close();
    QDomElement root = this->driver_xml.documentElement(); //根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    if(node.isNull())
        return ;

    QDomNode match_node = node.firstChild(); //获得第一个子节点

    if(match_node.isNull())
        return ;


    int display_num = match_node.toElement().text().toInt();
    TLOG_INFO("%d\n", display_num);
    match_node = match_node.nextSibling();

    int cur_no = 0;
    ACTION_DRIVER_KEY_INFO_S action_param;
    QPair<QString, QString> pair;
    DRIVER_CHN_S driver_chn;
    QString key_name;
    QString action_name;
    int pos;
    while(!match_node.isNull())
    {
        if(++cur_no > display_num)
            break ;
        QDomNode display_node = match_node.firstChild();
        key_name = display_node.toElement().text();
        display_node = display_node.nextSibling();
        for(pos = 0; pos < g_key_action_match.size(); pos++)
        {
            if(g_key_action_match[pos].first == key_name)
            {
                display_node.firstChild().setNodeValue(g_key_action_match[pos].second);
                display_node = display_node.nextSibling();
                QMap<QString,DRIVER_CHN_S>::const_iterator it = g_action_driver_match.constBegin();
                while(it != g_action_driver_match.constEnd())
                {
                    if(it.key() != g_key_action_match[pos].second){
                        it++;
                        continue;
                    }
                    display_node.firstChild().setNodeValue(it.value().driver1[0]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver1[1]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver1[2]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver1[3]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver2[0]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver2[1]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver2[2]);
                    display_node = display_node.nextSibling();
                    display_node.firstChild().setNodeValue(it.value().driver2[3]);
                    display_node = display_node.nextSibling();
                    it++;
                }
                break;
            }
        }
        match_node = match_node.nextSibling();
    }

    if(access(USED_XML_PATH, F_OK) == 0)
        unlink(USED_XML_PATH);
    file.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream out_stream(&file);
    out_stream.setCodec("utf-8");
    this->driver_xml.save(out_stream, 4); //缩进4格
    file.close();
    return ;
}

void driver_chn_config::save_slot(int flag)
{
    QString msg = "config -update ";
    TLOG_WARN("save_slot : %d", flag);
    if (flag == 1) {

        update_driver_chn_config_xml();
        driver_sync->load_xml(USED_XML_PATH);
        return ;
    }

    if(flag == 0)
    {

    }else if (flag == 2) {
        SyncParaFile *spf = new SyncParaFile(TYPE_DRIVER);
        spf->show();
    }else if (flag == 3) {
        //LOG(WARNING)<<"recover driver_chn_config.xml:"<<time(nullptr);
        TLOG_WARN("recover driver_chn_config.xml");

        if(access(USED_XML_PATH, F_OK) == 0)
            unlink(USED_XML_PATH);
        QFile::copy(BACK_XML_PATH, USED_XML_PATH);
        driver_sync->load_xml(USED_XML_PATH);
    }
    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
    this->close();
    this->~driver_chn_config();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

void driver_chn_config::update_slot()
{
    //TLOG_INFO(" update_slot \n");
    if(update_param_flag != true)
        update_param_flag = true;
    int tab = 0;

    QPair<QString, FAULT_INFO_S> pair;
    pair.first = "no_fault";
    pair.second.fault_time = 0;
    pair.second.fault_display = "系统无故障";
    pair.second.fault_diagnosis = "未检测到故障";

    struct tableViewArr arr = this->table_view_arr_list.at(tab);
    this->display_list_size = g_key_action_match.size();

    QStandardItemModel* model = nullptr;

    model = (QStandardItemModel*)arr.tableView_left->model();
    model->removeRows(0, model->rowCount());

    QFont font;
    QList<QPair<QString,QString>>::const_iterator it = g_key_action_match.constBegin();
    int i = 0;
    int row = 0;
    int index;
    QString display_chn;
    DRIVER_CHN_S temp;
    if(g_key_action_match.size() == 0)
    {
        TLOG_INFO("action.xml is null");
        return;
    }
    while (it != g_key_action_match.constEnd()) {
        display_chn.clear();
        model->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
        model->setItem(row, 1, new QStandardItem(it->second)); //设置tablebiew中的文字内容
        if(g_action_driver_match.find(it->second) != g_action_driver_match.end())
            temp = g_action_driver_match.find(it->second).value();
        display_chn+="S1:";
        for(index = 0; index < 4; index++){
            if(temp.driver1[index] != "NONE")
            {
                display_chn += temp.driver1[index];
                display_chn += " ";
            }
        }
        display_chn+=";S2:";
        for(index = 0; index < 4; index++){
            if(temp.driver2[index] != "NONE")
            {
                display_chn += temp.driver2[index];
                display_chn += " ";
            }
        }
        model->setItem(row, 2, new QStandardItem(display_chn)); //设置tablebiew中的文字内容
        model->item(row, 0)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
        model->item(row, 1)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
        model->item(row, 2)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示

        font = model->item(row, 0)->font();
        font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加
        model->item(row, 0)->setFont(font);
        model->item(row, 1)->setFont(font);
        model->item(row, 2)->setFont(font);

        if(++i > g_key_action_match.size())
        {
            break;
        }
        row++;
        it++;
    }
    this->table_view_arr_list[tab].tableView_left->selectRow(this->row);
}

void driver_chn_config::load_config()
{
        int tab = 0;
        if(g_key_action_match.isEmpty() == false)
            g_key_action_match.clear();
        if(g_action_driver_match.isEmpty() == false)
            g_action_driver_match.clear();
        QString file_name = "/userdata/config/hmi_files/driver_chn_config.xml";
        load_xml(file_name);
        QPair<QString, FAULT_INFO_S> pair;
        pair.first = "no_fault";
        pair.second.fault_time = 0;
        pair.second.fault_display = "系统无故障";
        pair.second.fault_diagnosis = "未检测到故障";

        struct tableViewArr arr = this->table_view_arr_list.at(tab);
        this->display_list_size = g_key_action_match.size();

        QStandardItemModel* model = nullptr;
        if(arr.tableView_left->model()){

            model = (QStandardItemModel*)arr.tableView_left->model();
            model->removeRows(0, model->rowCount());
        }else {
            model = new QStandardItemModel();
            m_ItemSelectionModel = new QItemSelectionModel(model);
            model->setColumnCount(3);
            model->setHeaderData(0, Qt::Horizontal, QString("按键"));
            model->setHeaderData(1, Qt::Horizontal, QString("动作名称"));
            model->setHeaderData(2, Qt::Horizontal, QString("通道"));

            arr.tableView_left->setModel(model);
            arr.tableView_left->setColumnWidth(0, 80);
            arr.tableView_left->setColumnWidth(1, 100);
            arr.tableView_left->setColumnWidth(2, 300);
            arr.tableView_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            arr.tableView_left->setSelectionModel(m_ItemSelectionModel);
            arr.tableView_left->setSelectionMode(QAbstractItemView::SingleSelection);  // 选择单项
            arr.tableView_left->setSelectionBehavior(QAbstractItemView::SelectRows);   // 选择整行
        }


        QFont font;
        QList<QPair<QString,QString>>::const_iterator it = g_key_action_match.constBegin();
        int i = 0;
        int row = 0;
        int index;
        QString display_chn;
        DRIVER_CHN_S temp;
        if(g_key_action_match.size() == 0)
        {
            TLOG_INFO("action.xml is null");
            return;
        }
        while (it != g_key_action_match.constEnd()) {
            display_chn.clear();
            model->setItem(row, 0, new QStandardItem(it->first)); //设置tablebiew中的文字内容
            model->setItem(row, 1, new QStandardItem(it->second)); //设置tablebiew中的文字内容
            temp = g_action_driver_match.find(it->second).value();
            display_chn+="S1:";
            for(index = 0; index < 4; index++){
                if(temp.driver1[index] != "NONE")
                {
                    display_chn += temp.driver1[index];
                    display_chn += " ";
                }
            }
            display_chn+=";S2:";
            for(index = 0; index < 4; index++){
                if(temp.driver2[index] != "NONE")
                {
                    display_chn += temp.driver2[index];
                    display_chn += " ";
                }
            }
            model->setItem(row, 2, new QStandardItem(display_chn)); //设置tablebiew中的文字内容
            model->item(row, 0)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
            model->item(row, 1)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示
            model->item(row, 2)->setTextAlignment(Qt::AlignLeft); //设置文字水平和垂直都居中显示

            font = model->item(row, 0)->font();
            font.setPointSize(14);   //设置tableview中文字的大小，如需设置其他可继续添加
            model->item(row, 0)->setFont(font);
            model->item(row, 1)->setFont(font);
            model->item(row, 2)->setFont(font);

            if(++i > g_key_action_match.size())
            {
                break;
            }
            row++;
            it++;
        }
        arr.tableView_left->clearSelection();
}

bool driver_chn_config::event(QEvent *event)
{
    QString strtab;

    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type()){
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
        case KeyValue::KEY_ARROW_LEFT:
        case KeyValue::KEY_ARROW_MINUS:
        case KeyValue::KEY_ARROW_PLUS:
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_UP:
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            if(this->row > 0)
                this->table_view_arr_list[tab].tableView_left->selectRow(--this->row);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            comm_acs_thread->control_timer->stop();
            if(this->row < (display_list_size - 1))
                this->table_view_arr_list[tab].tableView_left->selectRow(++this->row);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            if(this->row != -1){
                driver_chn_config_child *as = new driver_chn_config_child(this->row);
                as->show();
                this->widget_stack->push(this);
                connect(as, SIGNAL(update_signal()), this, SLOT(update_slot()));
                this->comm_acs_thread->cmdManager->window = as;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            if(update_param_flag){
                driver_chn_set *as = new driver_chn_set();
                as->show();
                this->widget_stack->push(this);
                connect(as, SIGNAL(save_signal(int)), this, SLOT(save_slot(int)));
                this->comm_acs_thread->cmdManager->window = as;
            }else {
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                this->~driver_chn_config();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        default:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;

    default:
        break;
    }


    return QWidget::event(event);
}
