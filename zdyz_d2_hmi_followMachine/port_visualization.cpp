#include "port_visualization.h"
#include "ui_port_visualization.h"
#include "Resources/key_value.h"
#include "parameter_config.h"

#include <QKeyEvent>
#include <QPainter>
#include <QDebug>

extern QVector<QMap<QString, FAULT_INFO_S>> g_fault;
extern int g_port_visualization;
int g_port_visualization = 0;

port_visualization::port_visualization(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::port_visualization)
{
    g_port_visualization = 1;
    ui->setupUi(this);
    widget_stack = WidgetStack::getInstance();
    comm_acs_thread = CommAcsSocketThread::getInstance();
    para = Parameters::getInstance();

    QFile file(":/qss/port_visualization.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("open qss file failed in %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        //this->setStyleSheet(ss);
        file.close();
    }
    ui->port1_button->setFocusPolicy(Qt::NoFocus);
    ui->port2_button->setFocusPolicy(Qt::NoFocus);
    ui->port3_button->setFocusPolicy(Qt::NoFocus);
    ui->port4_button->setFocusPolicy(Qt::NoFocus);
    ui->port5_button->setFocusPolicy(Qt::NoFocus);
    ui->port6_button->setFocusPolicy(Qt::NoFocus);
    ui->port7_button->setFocusPolicy(Qt::NoFocus);
    ui->port8_button->setFocusPolicy(Qt::NoFocus);
    ui->port9_button->setFocusPolicy(Qt::NoFocus);
    ui->port10_button->setFocusPolicy(Qt::NoFocus);
    ui->port11_button->setFocusPolicy(Qt::NoFocus);
    ui->port12_button->setFocusPolicy(Qt::NoFocus);
    ui->port_l_button->setFocusPolicy(Qt::NoFocus);
    ui->port_r_button->setFocusPolicy(Qt::NoFocus);
    ui->pushButton->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_2->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_3->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_4->setFocusPolicy(Qt::NoFocus);

    ui->label_L1->setStyleSheet("border: 1px solid green; color : white;");
    ui->label_L1->setAlignment(Qt::AlignCenter);

    m_setting_fault_map.insert("倾角", {"SensorAnglefault1", DISCONNECT_TYPE});
    m_setting_fault_map.insert("倾角", {"AngleOffLimit1", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("压力", {"SensorPressurefault2", DISCONNECT_TYPE});
    m_setting_fault_map.insert("压力", {"PressureOffLimit2", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("高度", {"SensorHighFault", DISCONNECT_TYPE});
    m_setting_fault_map.insert("高度", {"HighOffLimit", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("信号", {"SignalSwhNoAccess", DISCONNECT_TYPE});
    m_setting_fault_map.insert("键盘", {"LayCoalBoardFault", DISCONNECT_TYPE});
    m_setting_fault_map.insert("SBC1", {"DriverNoAccess", DISCONNECT_TYPE});
    m_setting_fault_map.insert("SBC1", {"DriverVoltOffLimit", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("SBC1", {"ConnectDriverfault", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("SBC1", {"DriverCpuFault", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("SBC2", {"DriverNoAccess2", DISCONNECT_TYPE});
    m_setting_fault_map.insert("SBC2", {"DriverVoltOffLimit2", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("SBC2", {"ConnectDriverfault2", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("SBC2", {"DriverCpuFault2", OFFLIMIT_TYPE});
    m_setting_fault_map.insert("反冲洗", {"UpbackFlushfault", DISCONNECT_TYPE});
    m_setting_fault_map.insert("反冲洗", {"DownbackFlushfault", DISCONNECT_TYPE});

    row = -1;
    column = 0;

    load_label_vector(m_label_vector);
    QList<info_prara> readonly_tabs;

    int size;
    QString name = "healthy";
    QList<QStringList> para_list = this->para->read(name, readonly_tabs, 1, list_index, size);

    for(int i=0; i<para_list[0].size(); i++){
        QStringList strlist = para_list[0][i].split(":");
        //DBG_ERR("%s %s\n", strlist[0].toUtf8().data(), strlist[1].toUtf8().data());
        QString port_str = strlist[1];

        if (!strlist[1].compare("禁用485模式")) {
            port_str = "禁用";
        }else if (!strlist[1].compare("数字倾角传感器模式")) {
            port_str = "倾角";
        }else if (!strlist[1].compare("数字压力传感器模式")) {
            port_str = "压力";
        }else if (!strlist[1].compare("数字高度传感器模式")) {
            port_str = "高度";
        }else if (!strlist[1].compare("信号转换器模式")) {
            port_str = "信号";
        }else if (!strlist[1].compare("放煤键盘模式")) {
            port_str = "键盘";
        }else if (!strlist[1].compare("电磁阀驱动器1模式")) {
            port_str = "SBC1";
        }else if (!strlist[1].compare("电磁阀驱动器2模式")) {
            port_str = "SBC2";
        }else if (!strlist[1].compare("反冲洗模式")) {
            port_str = "反冲洗";
        }else if(!strlist[1].compare("AD模式")){
            port_str = "AD";
        }
        switch(i){
        case 0:
            ui->label_P4->setText(port_str);
            break;
        case 1:
            ui->label_P5->setText(port_str);
            break;
        case 2:
            ui->label_P6->setText(port_str);
            break;
        case 3:
            ui->label_P7->setText(port_str);
            break;
        case 4:
            ui->label_P8->setText(port_str);
            break;
        default:
            break;
        }
        QString label_name = "P";
        label_name += QString::number(i);
        SET_STRATEGY_S unit = m_label_vector.at(1).at(i<3?i:i+1);
        /*delete the label match faults*/
        for(QMap<QString, QPair<STATUS_E, SET_STRATEGY_S>>::iterator it = m_fault_map.begin(); it != m_fault_map.end(); )
        {
            if(it.value().second.label->text().compare(label_name))
            {
                it++;
                continue;
            }
            m_fault_map.erase(it++);
        }

        /*match the label match faults*/
        QList<QPair<QString, STATUS_E>> value = m_setting_fault_map.values(port_str);/*find the setting match fault and type*/
        for(int i=0; i<value.size(); i++)
            m_fault_map.insert(value.at(i).first, {value.at(i).second, unit});
    }

    /*load faults*/
    int i;
    for(i=0; i<g_fault.size(); i++)
    {
        for(QMap<QString, FAULT_INFO_S>::iterator it = g_fault[i].begin(); it != g_fault[i].end(); it++)
        {
            update_status(it.key(), "1");
        }
    }
}

port_visualization::~port_visualization()
{
    g_port_visualization = 0;
    QString msg = QString("config -m 1");
    this->comm_acs_thread->send_socket_msg_to_acs(msg);
    delete ui;
}

void port_visualization::load_label_vector(QVector<QVector<SET_STRATEGY_S>> &label_vector)
{
    label_vector.resize(2);

    SET_STRATEGY_S policy;
    policy.label_unfocus = "background-color: rgb(0, 0, 60);border: 1px solid green; color : white;";
    policy.button_unfocus = "background_color: rgb(255, 255, 255);";

    policy.label = ui->label_P3;
    policy.button = ui->port3_button;
    policy.label_focus = "background-color:grey;";
    policy.line.push_back(ui->line_P3_1);
    policy.line.push_back(ui->line_P3_2);
    label_vector[0].push_back(policy);
    m_fault_map.insert("SensorInfrafault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("InfraOffLimit", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_P2;
    policy.button = ui->port2_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P2_1);
    policy.line.push_back(ui->line_P2_2);
    label_vector[0].push_back(policy);
    m_fault_map.insert("SensorPressurefault1", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("PressureOffLimit1", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_P1;
    policy.button = ui->port1_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P1);
    label_vector[0].push_back(policy);
    m_fault_map.insert("SensorTripfault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("TripOffLimit", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_L1;
    policy.button = ui->port_l_button;
    policy.line.clear();
    policy.line.push_back(ui->line_L1);
    label_vector[0].push_back(policy);
    m_fault_map.insert("LeftNetfault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("LeftCanfault", {OFFLIMIT_TYPE, policy});
    m_fault_map.insert("AllNetfault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("AllCanfault", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_P12;
    policy.button = ui->port12_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P12);
    label_vector[0].push_back(policy);

    policy.label = ui->label_P11;
    policy.button = ui->port11_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P11_1);
    policy.line.push_back(ui->line_P11_2);
    label_vector[0].push_back(policy);
    m_fault_map.insert("SensorNearFault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("NearOffLimit", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_P10;
    policy.button = ui->port10_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P10_1);
    policy.line.push_back(ui->line_P10_2);
    label_vector[0].push_back(policy);

    policy.label = ui->label_P4;
    policy.button = ui->port4_button;
    policy.label_focus = "background-color:yellow;";
    policy.line.clear();
    policy.line.push_back(ui->line_P4_1);
    policy.line.push_back(ui->line_P4_2);
    label_vector[1].push_back(policy);

    policy.label = ui->label_P5;
    policy.button = ui->port5_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P5_1);
    policy.line.push_back(ui->line_P5_2);
    label_vector[1].push_back(policy);

    policy.label = ui->label_P6;
    policy.button = ui->port6_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P6_1);
    label_vector[1].push_back(policy);

    policy.label = ui->label_R1;
    policy.button = ui->port_r_button;
    policy.label_focus = "background-color:grey;";
    policy.line.clear();
    policy.line.push_back(ui->line_R1_1);
    label_vector[1].push_back(policy);
    m_fault_map.insert("RightNetfault", {DISCONNECT_TYPE, policy});
    m_fault_map.insert("RightCanfault", {OFFLIMIT_TYPE, policy});
    m_fault_map.insertMulti("AllNetfault", {DISCONNECT_TYPE, policy});
    m_fault_map.insertMulti("AllCanfault", {OFFLIMIT_TYPE, policy});

    policy.label = ui->label_P7;
    policy.label_focus = "background-color:yellow;";
    policy.button = ui->port7_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P7_1);
    label_vector[1].push_back(policy);

    policy.label = ui->label_P8;
    policy.button = ui->port8_button;
    policy.line.clear();
    policy.line.push_back(ui->line_P8_1);
    policy.line.push_back(ui->line_P8_2);
    label_vector[1].push_back(policy);

    policy.label = ui->label_P9;
    policy.button = ui->port9_button;
    policy.label_focus = "background-color:grey;";
    policy.line.clear();
    policy.line.push_back(ui->line_P9_1);
    policy.line.push_back(ui->line_P9_2);
    label_vector[1].push_back(policy);
    return ;
}

void port_visualization::set_status(SET_STRATEGY_S icon, STATUS_E type)
{
    QString button_style_sheet = "border-radius: ";
    if(!icon.button->text().compare("L1") || !icon.button->text().compare("R1"))
        button_style_sheet += "20px;";
    else
        button_style_sheet += "15px;";

    switch(type){
    case CONNECTING_TYPE:
    case INIT_TYPE:
        button_style_sheet += "background-color: rgb(255, 255, 255);";
        icon.button->setStyleSheet(button_style_sheet);
        break;
    case CONNECTED_TYPE:
        button_style_sheet += "background-color: rgb(0, 255, 0);";
        icon.button->setStyleSheet(button_style_sheet);
        break;
    case DISCONNECT_TYPE:
        button_style_sheet += "background-color: rgb(255, 255, 0);";
        icon.button->setStyleSheet(button_style_sheet);
        break;
    case OFFLIMIT_TYPE:
        button_style_sheet += "background-color: rgb(255, 0, 0);";
        icon.button->setStyleSheet(button_style_sheet);
        break;
    default:
        break;
    }
}

void port_visualization::update_status(QString fault_name, QString fault_status)
{
    if(m_fault_map.find(fault_name) == m_fault_map.end()){
        return ;
    }
    QList<QPair<STATUS_E, SET_STRATEGY_S>> match_units = m_fault_map.values(fault_name);
    int i;
    for(i=0; i<match_units.size(); i++)
    {
        if(fault_status.contains("0"))
            set_status(match_units.at(i).second, CONNECTED_TYPE);
        else
            set_status(match_units.at(i).second, match_units.at(i).first);
    }
    return ;
}

void port_visualization::draw()
{

}

void port_visualization::update_label(int index)
{
    int size;
    QString name = "healthy";
    QList<info_prara> readonly_tabs;
    QList<QStringList> para_list = this->para->read(name, readonly_tabs, 1, list_index, size);

    QStringList strlist = para_list[0][index].split(":");
    QString port_str = strlist[1];

    if (!strlist[1].compare("禁用485模式")) {
        port_str = "禁用";
    }else if (!strlist[1].compare("数字倾角传感器模式")) {
        port_str = "倾角";
    }else if (!strlist[1].compare("数字压力传感器模式")) {
        port_str = "压力";
    }else if (!strlist[1].compare("数字高度传感器模式")) {
        port_str = "高度";
    }else if (!strlist[1].compare("信号转换器模式")) {
        port_str = "信号";
    }else if (!strlist[1].compare("放煤键盘模式")) {
        port_str = "键盘";
    }else if (!strlist[1].compare("电磁阀驱动器1模式")) {
        port_str = "SBC1";
    }else if (!strlist[1].compare("电磁阀驱动器2模式")) {
        port_str = "SBC2";
    }else if (!strlist[1].compare("反冲洗模式")) {
        port_str = "反冲洗";
    }else if(!strlist[1].compare("AD模式")){
        port_str = "AD";
    }
    switch(index){
    case 0:
        ui->label_P4->setText(port_str);
        break;
    case 1:
        ui->label_P5->setText(port_str);
        break;
    case 2:
        ui->label_P6->setText(port_str);
        break;
    case 3:
        ui->label_P7->setText(port_str);
        break;
    case 4:
        ui->label_P8->setText(port_str);
        break;
    default:
        break;
    }

    QString label_name = "P";
    label_name += QString::number(index);
    SET_STRATEGY_S unit = m_label_vector.at(1).at(index<3?index:index+1);
    /*delete the label match faults*/
    for(QMap<QString, QPair<STATUS_E, SET_STRATEGY_S>>::iterator it = m_fault_map.begin(); it != m_fault_map.end(); )
    {
        if(it.value().second.label->text().compare(label_name))
        {
            it++;
            continue;
        }
        m_fault_map.erase(it++);
    }

    /*match the label match faults*/
    QList<QPair<QString, STATUS_E>> value = m_setting_fault_map.values(port_str);/*find the setting match fault and type*/
    for(int i=0; i<value.size(); i++)
        m_fault_map.insert(value.at(i).first, {value.at(i).second, unit});
}

bool port_visualization::event(QEvent *event)
{
    QString strtab;
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type()){
    case KeyValue::KEY_PRESSED:
        this->comm_acs_thread->cmdManager->lock_ui_off();
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:{
            if(row < 0)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            if(column == 1)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_unfocus);
            column = 1;
            m_label_vector.at(column).at(row).button->setFocus();
            //m_label_vector.at(column).at(row).button->setStyleSheet("border: 1px solid yellow;border-radius: 15px;background-color: red");
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_focus);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_LEFT:{
            if(row < 0)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            if(column == 0)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_unfocus);
            column = 0;
            m_label_vector.at(column).at(row).button->setFocus();
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_focus);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_UP:{
            if(row <= 0)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_unfocus);
            m_label_vector.at(column).at(--row).button->setFocus();
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_focus);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_DOWN:{
            if(row ==  m_label_vector.at(column).size()-1)
            {
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            if(row != -1)
                m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_unfocus);
            m_label_vector.at(column).at(++row).button->setFocus();
            m_label_vector.at(column).at(row).label->setStyleSheet(m_label_vector.at(column).at(row).label_focus);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_OK:{
            QString strtab = "healthy";
            if(column == 0){
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            if((this->row == 3)||(this->row == 6)){
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }

            QString write_access = "write";
            int index = row<3?row:(row-1);
            if(this->para->getAccess(strtab, 1, list_index[0][index], write_access) == 1)//当有权限时打开
            {
                //DBG_ERR("%d-%d\n", index, list_index[0][index]);
                g_port_visualization = 2;
                ParameterConfig *para_config = new ParameterConfig(strtab, 1, index, list_index[0][index]);
                para_config->show();
                this->widget_stack->push(this);
                this->comm_acs_thread->cmdManager->window = para_config;

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_CANCEL:{
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~port_visualization();
            //m_painter->close();
            //delete m_painter;
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



