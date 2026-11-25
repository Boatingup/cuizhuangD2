#include "action_set.h"
#include "ui_action_set.h"

#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QTextCodec>
#include <QByteArray>

#include <mutex>

#include "./window/leftpane.h"
#include "./Resources/parameters.h"
#include "threads/comm_acs_thread.h"
#include "./Resources/error_code.h"
#include "./Resources/mqtt_message.h"
#include "actionconfigure.h"
#include "./Resources/key_value.h"

extern std::mutex action_mutex;

int action_set::getActionByindex(int index)
{
    if(index > action_register->action_list.size() + 1)
    {
        action_key = "none";
        action_value = "none";
        return -1;
    }

    QList<QPair<QString,Action_info>>::const_iterator it = action_register->action_list.constBegin();
    int i = 0;
    if(index == 0) {
        action_key = it->first;
        action_value = it->second.action_name;
        return 0;
    } else {
        while (it != action_register->action_list.constEnd()) {
            ++it;
            i++;
            if(i == index)
            {
                action_key = it->first;
                action_value = it->second.action_name;
                return 0;
            }
        }
    }
    return 0;
}

action_set::action_set(int index,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::action_set)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    action_index = index;
    QFile file(":/qss/action_set.qss");
    if(!file.open(QFile::ReadOnly)){
       TLOG_INFO("open qss file failed in menuPage().");
    }else{
        QString styleSheet = file.readAll();
        this->setStyleSheet(styleSheet);
        file.close();
    }

    this->str_temp = "";

    this->is_number = 1;
    this->parameter_type = 0;
    this->high = 255;
    this->low = 0;
    this->is_global = 1;

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->pst = ParaSynchronizeThread::getInstance();

    this->move(60, 16);
    //this->str_category = category_name;
    //this->tab = tab;
    //this->row = row;

    this->para = Parameters::getInstance();

    this->str_send = "";
    this->ok_step = 0;

    this->action_register = ActionRegister::getInstance();
    int rtn = this->action_register->getActionByindex(action_index,action_key,action_value,english_action);

    //QString key_index;
    //QString Value_index;
    //int rtn = getActionByindex(action_index);
    if(rtn == 0)
    {
        ui->label_parameter_name->setText(action_value);
        ui->label_input_value->setText(action_key);
    }
    else {
        TLOG_INFO("error find actions \n");
    }


    /*
    int size = this->action_register->action_map.size();
    QHash<QString,QString>::const_iterator it = action_register->action_map.constBegin();
    while (it != action_register->action_map.constEnd()) {
        qDebug() << it.key() << ": " << it.value();
        ++it;
    }
    */

}

action_set::~action_set()
{
    delete ui;
}


bool action_set::IsNumBetween(float num)
{
    return(num <= high && num >= low);
}
bool action_set::IsNumNotinSide(float num)
{
    return(num <= high);
}

bool action_set::event(QEvent *event)
{
    QString str_label = "real_value";
    //QString str_parameter_name;
    QRegExp regExp("^?(\\d{1,2}|[1]\\d{2}|2[0-4]\\d|255)$");//0 到255的整数
    QString msg;

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
        this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->stop();
            if((this->parameter_type == 0) && (str_temp.size() > 0)){
                str_temp = str_temp.mid(0, str_temp.size()-1);
                ui->label_input_value->setText(str_temp);
            }
        this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_UP:
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->stop();
            if(this->parameter_type == 1){
                QStringList list = this->para->query_range_set(this->str_category, this->tab, this->parameter_name);
                //list.size();

                this->list_range_set_index++;
                if(this->list_range_set_index >= list.size()){
                    this->list_range_set_index = 0;
                }
                this->new_set_value = QString::number(this->list_range_set_index);
                ui->label_input_value->setText(list[list_range_set_index]);
                //this->new_set_value = list[list_range_set_index];
                //ui->label_input_value->setText(this->new_set_value);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();

            //std::lock_guard<std::mutex> lck(action_mutex);
            if(this->ok_step == 0){

                TLOG_INFO("parameter_type = %d", this->parameter_type);
                TLOG_INFO("str_temp = %d", str_temp.isEmpty());

                if(this->parameter_type == 0){ //numberic type
                    if(!this->str_temp.isEmpty()){
                        QString temp =  str_temp;
                        //str_temp = QString::number(temp.toFloat() + 0.0);
                        if(temp.at(temp.size() - 1) == '.')
                        {
                            str_temp += "0";
                        }

                        //str_parameter_name = this->para->query_parameter_name(this->str_category, this->tab, this->row);
                        if(IsNumBetween(str_temp.toFloat())==false)
                        {
                            //ui->label_2->setText("数值不在范围内");
                            //ui->label->setText(" ");
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            this->~action_set();
                            static_cast<ActionConfigure *>(this->comm_acs_thread->cmdManager->window)->load_actions(tab,row);

                        }
                        else
                        {
                            QString str_path = HMI_FILES_PATH_BASE;
                            QString file = str_path;
                            file += "/depend_files/action.xml";
                            QString name = english_action;
                            QString label = "id";
                            QString value = str_temp;
                            QString action = "action";
                            if(this->action_register->update(file, name,label, value) != 0)
                            {
                                  this->comm_acs_thread->cmdManager->lock_ui_off();
                                  return QWidget::event(event);
                            }

                            str_send = action + ":" + english_action +":"+ str_temp;
                            qDebug() << "++++ action: " << str_send;

                            if(this->is_global){
                                this->comm_acs_thread->cmdManager->is_self_action_update = true;

                                this->pst->send_para_synchronization_msg(str_send);
                                QJsonObject json;
                                json["reload"] = "action.xml";
                                QJsonDocument jsonDoc(json);
                                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());
                                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                                this->close();
                                this->~action_set();
                                this->action_register->reload();

                                static_cast<ActionConfigure *>(this->comm_acs_thread->cmdManager->window)->load_actions(tab,action_index);
                                //static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,row);
                            }else{
                                this->ok_step = 1;
                                ui->label->setText("同步按确认");
                                ui->label_2->setText("不同步按取消");
                                TLOG_INFO("hmi:on ok_step 1");
                            }
                        }
                    }else{
                        this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                        this->close();
                        this->~action_set();
                    }
                }else{ //switch type

                }
            }else if(this->ok_step == 1){
                this->pst->send_para_synchronization_msg(str_send);
                sync();
                QJsonObject json;
                json["reload"] = "action.xml";
                QJsonDocument jsonDoc(json);
                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                this->~action_set();
                this->action_register->reload();
                static_cast<ActionConfigure *>(this->comm_acs_thread->cmdManager->window)->load_actions(tab,action_index);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:

            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            //std::lock_guard<std::mutex> lck(action_mutex);
            if(this->ok_step == 1){

                this->action_register->reload();
                static_cast<ActionConfigure *>(this->comm_acs_thread->cmdManager->window)->load_actions(tab,action_index);
                //static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,row);
            }

            this->~action_set();
            //this->comm_acs_thread->cmdManager->lock_ui_off();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->stop();
            if(this->parameter_type == 0){
                QString mytemp = str_temp;

                //if((mytemp == ".")|| (str_temp.indexOf(".") >= 0))

                if(IsNumNotinSide((mytemp += "0").toFloat()))
                {

                    if(mytemp == "00")
                    {

                    }
                    else {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "0";
                        }
                    }

                }
                ui->label_input_value->setText(str_temp);

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
                if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "1").toFloat()))
                    {

                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "1";
                        }
                    }
                }
                ui->label_input_value->setText(str_temp);
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
                if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "2").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "2";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "3").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "3";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "4").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "4";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "5").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "5";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "6").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "6";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "7").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "7";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "8").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "8";
                        }
                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->stop();
            if(this->ok_step == 0)
            {
            if(this->parameter_type == 0){
                QString mytemp = str_temp;
                if(mytemp != "0")
                {
                    if(IsNumNotinSide((mytemp += "9").toFloat()))
                    {
                        if((regExp.exactMatch(mytemp)) == 1)
                        {
                            str_temp += "9";
                        }

                    }
                    ui->label_input_value->setText(str_temp);
                }
            }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F1: //used as "."
            comm_acs_thread->control_timer->stop();
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
