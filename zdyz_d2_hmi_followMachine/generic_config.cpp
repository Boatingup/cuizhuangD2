#include "generic_config.h"
#include "ui_generic_config.h"

#include <QEvent>
#include <QKeyEvent>
#include <unistd.h>
#include "Resources/mqtt_message.h"

extern QList<sensor_info> sensor_list;

generic_config::generic_config(GENERIC_CONFIG_E type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::generic_config)
{
    ui->setupUi(this);
    this->type = type;
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->row = 0;  //光标所在行,-1是为了按向下按键时+1后为0，对应list中下标为0
    this->column = 0;//光标所在列， 0为左列，1为右列
    switch(this->type){
    case TYPE_GENERIC_CONNECT_BOARD:{
        ui->title_label->setText("总电压电流超限配置");
        ui->volt_lineEdit->setPlaceholderText("V");
        ui->cur_lineEdit->setPlaceholderText("mA");
        ui->volt_label->setText("总电压下限(V)");
        ui->cur_label->setText("总电流上限(mA)");
        int i;
        QString temp;
        for(i=0; i<sensor_list.size(); i++)
        {
            if(sensor_list[i].name.contains("ConnectTotalVolt"))
            {
                ui->volt_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].min)));
            }
            else if(sensor_list[i].name.contains("ConnectTotalCur"))
            {
                ui->cur_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].max)));
            }
        }
        ui->volt_lineEdit->setFocus();
        break;
    }
    case TYPE_GENERIC_SENSOR_ANGLE:{
        ui->title_label->setText("倾角超限配置");
        ui->volt_label->setText("角度上限值设置");
        ui->cur_label->setText("角度下限值设置");
        int i;
        QString temp;
        for(i=0; i<sensor_list.size(); i++)
        {
            if(sensor_list[i].name.contains("Inclinationsensor1"))
            {
                ui->volt_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].max)));
                ui->cur_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].min)));
            }
        }
        ui->volt_lineEdit->setFocus();
        break;
    }
    case TYPE_GENERIC_SENSOR_HIGH:{
        ui->title_label->setText("高度超限配置");
        ui->volt_label->setText("高度上限值设置(mm)");
        ui->cur_label->setText("高度下限值设置(mm)");
        int i;
        QString temp;
        for(i=0; i<sensor_list.size(); i++)
        {
            if(sensor_list[i].name.contains("HighSensor"))
            {
                ui->volt_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].max)));
                ui->cur_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].min)));
            }
        }
        ui->volt_lineEdit->setFocus();
        break;
    }
    case TYPE_GENERIC_DRIVER_VOLT:{
        ui->title_label->setText("驱动器电压超限配置");
        ui->volt_label->setText("电压上限值设置(V)");
        ui->volt_label->hide();
        ui->volt_lineEdit->hide();
        ui->cur_label->setText("电压下限值设置(V)");

        ui->cur_label->setGeometry(50,130,120,26);
        ui->cur_lineEdit->setGeometry(210,130,113,26);
        this->row = 1;
        int i;
        QString temp;
        for(i=0; i<sensor_list.size(); i++)
        {
            if(sensor_list[i].name.contains("DriverVolt")&&(!sensor_list[i].name.contains("DriverVolt2")))
            {
                ui->volt_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].max)));
                ui->cur_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].min)));
            }
        }
        ui->cur_lineEdit->setFocus();
        break;
    }
    case TYPE_GENERIC_DRIVER_CUR:{
        ui->title_label->setText("驱动器电流超限配置");
        ui->volt_label->setText("电流上限值设置(mA)");
        ui->cur_label->setText("电流下限值设置(mA)");
        ui->cur_label->hide();
        ui->cur_lineEdit->hide();
        ui->volt_label->setGeometry(50,130,120,26);
        ui->volt_lineEdit->setGeometry(210,130,113,26);
        int i;
        QString temp;
        for(i=0; i<sensor_list.size(); i++)
        {
            if(sensor_list[i].name.contains("DriverCur")&&(!sensor_list[i].name.contains("DriverCur2")))
            {
                ui->volt_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].max)));
                ui->cur_lineEdit->setText(QString::number(static_cast<double>(sensor_list[i].min)));
            }
        }
        ui->volt_lineEdit->setFocus();
        break;
    }
    }

}

generic_config::~generic_config()
{
    delete ui;
}

void generic_config::update_driver_chn_config_xml(QString volt, QString cur)
{
    TLOG_INFO("%s-%s\n", volt.toUtf8().data(), cur.toUtf8().data());
    QString file_path="/userdata/config/hmi_files/parameters.xml";
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("hmi:open para xml file failed.");
        return ;
    }
    QDomDocument driver_xml;
    if(!driver_xml.setContent(&file))
    {
        file.close();
        TLOG_ERROR("hmi: Dom set para xml content failed.");
        return ;
    }
    file.close();
    QDomElement root = driver_xml.documentElement(); //根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    while(!node.isNull())
    {
        if(!node.toElement().attribute("category").compare("healthy"))
            break;
        node = node.nextSibling();
    }
    if(node.isNull())
        return ;

    QDomNode match_node = node.firstChild(); //获得第一个子节点

    while(!match_node.isNull())
    {
        if(!match_node.toElement().attribute("name").compare("SenSorData"))
            break;
        match_node = match_node.nextSibling();
    }
    if(match_node.isNull())
        return ;

    QDomNode element_node = match_node.firstChild();
    while(!element_node.isNull())
    {
        QDomNode element_child_node = element_node.firstChild();
        if(this->type == TYPE_GENERIC_CONNECT_BOARD){
            if(!element_node.toElement().attribute("name").compare("ConnectTotalVolt"))
            {
                while(!element_child_node.isNull())
                {
                    if(!element_child_node.nodeName().compare("range_low"))
                        break;
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
                element_child_node.firstChild().setNodeValue(volt);
            }
            else if(!element_node.toElement().attribute("name").compare("ConnectTotalCur"))
            {
                while(!element_child_node.isNull())
                {
                    if(!element_child_node.nodeName().compare("range_high"))
                        break;
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
                element_child_node.firstChild().setNodeValue(QString::number(cur.toDouble()));
            }
        }else if(this->type == TYPE_GENERIC_SENSOR_ANGLE){
            if(!element_node.toElement().attribute("name").compare("Inclinationsensor1"))
            {
                while(!element_child_node.isNull())
                {
                    if(!element_child_node.nodeName().compare("range_high")){
                        element_child_node.firstChild().setNodeValue(volt);
                        break;
                    }else if(!element_child_node.nodeName().compare("range_low")){
                        element_child_node.firstChild().setNodeValue(cur);
                    }
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
            }
        }else if(this->type == TYPE_GENERIC_SENSOR_HIGH){
            if(!element_node.toElement().attribute("name").compare("HighSensor"))
            {
                while(!element_child_node.isNull())
                {
                    if(!element_child_node.nodeName().compare("range_high")){
                        element_child_node.firstChild().setNodeValue(volt);
                        break;
                    }else if(!element_child_node.nodeName().compare("range_low")){
                        element_child_node.firstChild().setNodeValue(cur);
                    }
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
            }
        }else if(this->type == TYPE_GENERIC_DRIVER_VOLT){
            if(!element_node.toElement().attribute("name").compare("DriverVolt"))
            {
                while(!element_child_node.isNull())
                {
                    //if(!element_child_node.nodeName().compare("range_high")){
                        //element_child_node.firstChild().setNodeValue(volt);
                        //break;
                    //}else if(!element_child_node.nodeName().compare("range_low")){
                    if(!element_child_node.nodeName().compare("range_low")){
                        element_child_node.firstChild().setNodeValue(cur);
                        break;
                    }
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
            }
        }else if(this->type == TYPE_GENERIC_DRIVER_CUR){
            if(!element_node.toElement().attribute("name").compare("DriverCur"))
            {
                while(!element_child_node.isNull())
                {
                    if(!element_child_node.nodeName().compare("range_high")){
                        element_child_node.firstChild().setNodeValue((QString::number(volt.toDouble())));
                        break;
                    }//else if(!element_child_node.nodeName().compare("range_low")){
                        //element_child_node.firstChild().setNodeValue((QString::number(cur.toFloat())));
                    //}
                    element_child_node = element_child_node.nextSibling();
                }
                if(element_child_node.isNull())
                    return ;
            }
        }
        element_node = element_node.nextSibling();
    }

    if(access(file_path.toUtf8().data(), F_OK) == 0)
        unlink(file_path.toUtf8().data());
    file.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream out_stream(&file);
    out_stream.setCodec("utf-8");
    driver_xml.save(out_stream, 4); //缩进4格
    file.flush();
    fsync(file.handle());
    file.close();

    QJsonObject json;
    json["reload"] = "parameters.xml";
    QJsonDocument jsonDoc(json);
    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());
    return ;
}

bool generic_config::event(QEvent *event)
{
    int num = -1;
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:
        break;
    case KeyValue::KEY_RELEASED:
    {
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->stop();
            if((this->type != TYPE_GENERIC_DRIVER_VOLT)&&(this->type != TYPE_GENERIC_DRIVER_CUR))
                if(++this->row > 1)
                    this->row = 1;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->stop();
            if((this->type != TYPE_GENERIC_DRIVER_VOLT)&&(this->type != TYPE_GENERIC_DRIVER_CUR))
                if(--this->row < 0)
                    this->row = 0;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_RIGHT:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
        {
            comm_acs_thread->control_timer->stop();

            QString volt = ui->volt_lineEdit->text();
            QString cur = ui->cur_lineEdit->text();
            int i;
            switch(this->type){
            case TYPE_GENERIC_CONNECT_BOARD:{
                for(i=0; i<sensor_list.size(); i++)
                {
                    if(sensor_list[i].name.contains("ConnectTotalVolt"))
                    {
                        sensor_list[i].min = volt.toFloat();
                    }
                    else if(sensor_list[i].name.contains("ConnectTotalCur"))
                    {
                        sensor_list[i].max = cur.toFloat();
                    }
                }
                break;
            }
            case TYPE_GENERIC_SENSOR_ANGLE:{
                for(i=0; i<sensor_list.size(); i++)
                {
                    if(sensor_list[i].name.contains("Inclinationsensor1"))
                    {
                        sensor_list[i].max = volt.toFloat();
                        sensor_list[i].min = cur.toFloat();
                    }
                }
                break;
            }
            case TYPE_GENERIC_SENSOR_HIGH:{
                for(i=0; i<sensor_list.size(); i++)
                {
                    if(sensor_list[i].name.contains("HighSensor"))
                    {
                        sensor_list[i].max = volt.toFloat();
                        sensor_list[i].min = cur.toFloat();
                    }
                }
                break;
            }
            case TYPE_GENERIC_DRIVER_VOLT:{
                for(i=0; i<sensor_list.size(); i++)
                {
                    if(sensor_list[i].name.contains("DriverVolt"))
                    {
                        sensor_list[i].max = volt.toFloat();
                        sensor_list[i].min = cur.toFloat();
                    }
                }
                break;
            }
            case TYPE_GENERIC_DRIVER_CUR:{
                for(i=0; i<sensor_list.size(); i++)
                {
                    if(sensor_list[i].name.contains("DriverCur"))
                    {
                        sensor_list[i].max = volt.toFloat();
                        sensor_list[i].min = cur.toFloat();
                    }
                }
                break;
            }
            default:
                break;
            }
            update_driver_chn_config_xml(volt, cur);
            goto next_state;
            break;
        }
        case KeyValue::KEY_CANCEL:
            next_state:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~generic_config();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_0:
            this->comm_acs_thread->control_timer->stop();
            num = 0;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_1:
            this->comm_acs_thread->control_timer->stop();
            num = 1;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_2:
            this->comm_acs_thread->control_timer->stop();
            num = 2;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_3:
            this->comm_acs_thread->control_timer->stop();
            num = 3;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_4:
            this->comm_acs_thread->control_timer->stop();
            num = 4;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_5:
            this->comm_acs_thread->control_timer->stop();
            num = 5;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_6:
            this->comm_acs_thread->control_timer->stop();
            num = 6;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_7:
            this->comm_acs_thread->control_timer->stop();
            num = 7;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_PUSH_PULL:
            this->comm_acs_thread->control_timer->stop();
            num = 8;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_9:
            this->comm_acs_thread->control_timer->stop();
            num = 9;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F1:
            this->comm_acs_thread->control_timer->stop();
            num = 10;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F2:
            this->comm_acs_thread->control_timer->stop();
            num = 11;
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F3:
        case KeyValue::KEY_F4:
        case KeyValue::KEY_F5:
        case KeyValue::KEY_F6:
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
    }
        break;
    default:
        return QWidget::event(event);
        //break;
    }
#if 0
    if(this->type == TYPE_GENERIC_DRIVER_CUR){
        this->row = 1;
    }else if (this->type == TYPE_GENERIC_DRIVER_VOLT) {
        this->row = 0;
    }
#endif
    if((key_event->key() == KeyValue::KEY_ARROW_DOWN)||(key_event->key() == KeyValue::KEY_ARROW_UP))
    {
        switch(this->row){
        case 0:
            ui->volt_lineEdit->setFocus();
            break;
        case 1:
            ui->cur_lineEdit->setFocus();
            break;
        default:
            break;
        }
    }
    if((key_event->key() != KeyValue::KEY_OK)&&(key_event->key() != KeyValue::KEY_CANCEL))
    {
        switch(this->row){
        case 0:
        {
            if(key_event->key() == KeyValue::KEY_ARROW_MINUS)
            {
                ui->volt_lineEdit->clear();
            }
            QString cur = ui->volt_lineEdit->text();
            if(num != -1)
            {
                if(num == 10)
                    cur += ".";
                else
                    cur += QString::number(num);
            }
            ui->volt_lineEdit->setText(cur);
            break;
        }
        case 1:
        {
            if(key_event->key() == KeyValue::KEY_ARROW_MINUS)
            {
                ui->cur_lineEdit->clear();
            }
            QString cur = ui->cur_lineEdit->text();
            if(num == 11)
            {
                cur += "-";
            }
            else if(num != -1)
            {
                cur += QString::number(num);
            }
            ui->cur_lineEdit->setText(cur);
            break;
        }
        default:
            break;
        }
    }
    return QWidget::event(event);
}
