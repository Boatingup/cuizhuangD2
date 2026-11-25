#include "parameter_config.h"
#include "port_visualization.h"
#include "ui_parameter_config.h"

#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QTextCodec>
#include <QByteArray>

#include "./Resources/key_value.h"
#include "./Resources/mqtt_message.h"
#include "./window/leftpane.h"
#include "./Resources/parameters.h"
#include "threads/comm_acs_thread.h"
#include "./Resources/error_code.h"
#include "dialog_usercheckandlogin.h"
//#include "./Resources/tcpclient.h"
#include "./window/mainpage.h"
#include "log_head.h"

#define BACK_GROUND_LIGHT "/sys/class/backlight/rgb-backlight/brightness"

int GroupVolume;
int video_frames;//视频帧率
int GroupVolumeDistance;//组播间隔
extern int g_coal_machine_fence;
//extern QList<info_prara> readonly_tabs;

extern QString syzk_user;
extern QList <mainPage_uigonfig> menuconfig_list;


extern leftPane* parameter_show;


extern int g_port_visualization;
ParameterConfig::ParameterConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParameterConfig)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    distinguish_flag = 1;
    this->setGeometry(150, 90, 500, 300);

    this->widget_stack = WidgetStack::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();

    connect(this,SIGNAL(change_user()),this->comm_acs_thread,SLOT(closeAllwindow()));

    ui->label_3->setFixedWidth(60);
    this->action_register = ActionRegister::getInstance();

    ui->label_parameter_name->setText("动作下发");

    ui->label_range_low->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
    delete ui->label_range_low;

    ui->label_range_high->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
    delete ui->label_range_high;

    ui->label_bolangxian->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
    delete ui->label_bolangxian;

    QString set("[");
    for(int i = 0; i< this->action_register->action_aditional_list.size() - 1;i++)
    {
        set += this->action_register->action_aditional_list[i].second.action_name;
        set += "/";
    }
    set += this->action_register->action_aditional_list[this->action_register->action_aditional_list.size() - 1].second.action_name;
    set += "]";
    ui->label_range_set->setText(set);
    ui->label_range_set->setFixedWidth(300);
    exact_size = action_register->action_aditional_list.size();
    ui->label_input_value->setText(this->action_register->action_aditional_list[exact_index].second.action_name);

    //setFont  无效   setstylesheet 可以
    ui->label->setStyleSheet("font-size:28px");
    ui->label_2->setStyleSheet("font-size:28px");
    ui->label->setText("按\"<-\"确定");
}

ParameterConfig::ParameterConfig(QString& category_name, int tab, int ui_index,int row, QWidget *parent):
    QDialog(parent),
    ui(new Ui::ParameterConfig)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(500,300);
    index_ui = ui_index;
    distinguish_flag = 0;

    this->str_temp = "";
    this->str_hour = "";
    this->str_min = "";

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->pst = ParaSynchronizeThread::getInstance();

    this->move(150, 90);
    this->str_category = category_name;
    this->tab = tab;
    this->row = row;

    this->para = Parameters::getInstance();

    this->str_send = "";
    this->ok_step = 0;

    QStringList list = para->read(category_name,tab,readonly_tabs,index_list);
    QStringList strlist = list[ui_index].split(":");

    ui->label_parameter_name->setText(strlist.at(0));
    ui->label_input_value->setText(strlist.at(1));

    this->parameter_name = this->para->query_parameter_name(category_name, tab, row);

    QString label = "range_set_num";
    QString label_real_value = "real_value";
    QString unit_label = "unit";
    QString typeLabel = "type";

    QString type = this->para->query(category_name, tab, parameter_name, typeLabel);
    QString num_str = this->para->query(category_name, tab, parameter_name, unit_label);
    if(num_str == "#" || num_str == " " )
    {
        this->is_number = 1;
    }
    else {
        this->is_number = 0;
    }
    TLOG_INFO("##### category_name = %s ,parameter_name = %s ", category_name.toUtf8().data(), parameter_name.toUtf8().data());
    QString default_value_label = "default_value";
    QString default_str = this->para->query(category_name, tab, parameter_name, default_value_label);

    if(default_str == "send")
    {
        this->send_flag = 1;
    } else {
        this->send_flag = 0;
    }
    qDebug() << "paadasdasdasdasde++send_flag+++ " << send_flag;
    QString str = this->para->query(category_name, tab, parameter_name, label);
    if (type == "ip") {
        parameter_type = 2;
        QString ip = this->para->query(category_name, tab, parameter_name, label_real_value);
        QStringList list = ip.split('.');
        if(list.size() == 4)
        {
            for(int i=0;i<4;i++)
            {
                ipSegments[i] = list[i];
            }
        }
        else
        {
            ipSegments[0]="192";
            ipSegments[1]="168";
            ipSegments[2]="1";
            ipSegments[3]="1";
        }

        ui->label_range_set->hide();
        ui->label_range_low->hide();
        ui->label_range_high->hide();
        ui->label_bolangxian->hide();
        updateIpDisplay();
    } else if(str == ""){
        parameter_type = 0; // numberic type
        label = "range_low";
        str = this->para->query(category_name, tab, parameter_name, label);
        if(IsFirstRecoilTime())
        {
            QStringList strlist = str.split(':');
            h_low = strlist[0].toInt();
            m_low = strlist[1].toInt();
        } else {
            low = str.toFloat();
        }

        ui->label_range_low->setText(str);

        label = "range_high";
        str = this->para->query(category_name, tab, parameter_name, label);
        if(IsFirstRecoilTime())
        {
            QStringList strlist = str.split(':');
            h_high = strlist[0].toInt();
            m_high = strlist[1].toInt();
        } else {
            high = str.toFloat();
        }

        ui->label_range_high->setText(str);
        ui->label_range_set->hide();
        ui->label_range_low->show();
        ui->label_range_high->show();
        ui->label_bolangxian->show();

        if(IsFirstRecoilTime())
        {
            QStringList strlist = ui->label_input_value->text().split('-');
            ui->label_input_value->setText(QString("%1:%2").arg(strlist[0]).arg(strlist[1]));
        }
    }else{
        parameter_type = 1; // switch type
        QStringList range_list;
        QStringList mode_list;
        if(send_flag == 1)
        {
              range_list = this->para->query_range_set(category_name, tab, parameter_name,mode_list);
              mode = mode_list[0].toLower();
        } else {
              range_list = this->para->query_range_set(category_name, tab, parameter_name);
        }
        QString set("[");
        for(int i=0; i<range_list.size()-1; i++){
            set += range_list[i];
            set += "/";
        }
        set += range_list[range_list.size()-1];
        set += "]";

        ui->label_range_set->setText(set);
        ui->label_range_set->setFixedWidth(300);
        ui->label_range_set->show();

        ui->label_range_low->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
        //ui->gridLayout->removeWidget(ui->label_range_low);
        delete ui->label_range_low;

        ui->label_range_high->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
        //ui->gridLayout->removeWidget(ui->label_range_high);
        delete ui->label_range_high;

        ui->label_bolangxian->setParent(nullptr);    //记得移除前将widget的parent置空，不然删不掉
        //ui->gridLayout->removeWidget(ui->label_bolangxian);
        delete ui->label_bolangxian;

        QString str_real_value = this->para->query(category_name, tab, parameter_name, label_real_value);
        //add code
        PortModeInfo info = para->queryPortModeInfo(category_name, tab, parameter_name);
        if (!info.baudrateList.empty()) {
            baudrateList = info.baudrateList;
            baudrateIndex = info.baudrateIndex;
            modeIndex = info.modeIndex;
            modeText = info.modeText;

            QString showText = modeText;
            if (baudrateIndex >= 0 && baudrateIndex < baudrateList.size())
            {
                showText += "  " + baudrateList.at(baudrateIndex);
            }
            ui->label_input_value->setText(showText);
        }

        this->list_range_set_index = str_real_value.toInt();
        this->new_set_value = str_real_value;

        ui->label_3->setFixedWidth(60);

    }

    label = "global";
    QString str_isglobal = this->para->query(category_name, tab, parameter_name, label);

    if(str_isglobal == "g_global"){
        this->is_global = 1; //global
    }else if(str_isglobal == "g_local"){
        this->is_global = 0; //local
    }
    else if(str_isglobal == "g_local_only"){
        this->is_global = -1; //local
    }

    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    TLOG_INFO("hmi:is_global is %d", this->is_global);
}

ParameterConfig::~ParameterConfig()
{
    delete ui;
    if(g_port_visualization == 2)
        g_port_visualization = 1;
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
}

void ParameterConfig::FirstRecoilTimeProc(QString value)
{
    QString hour_tmp = str_hour;
    QString min_tmp = str_min;
    if(str_hour.size() < 2)
    {
        if(str_hour.size() < 1)
        {
            if((hour_tmp += value).toInt() <= 2)
            {
                str_hour += value;
            }
        } else {
            if((hour_tmp += value).toInt() <= 23)
            {
                str_hour += value;
            }
        }
    } else if(str_min.size() < 2){
        if(str_min.size() < 1)
        {
            if((min_tmp += value).toInt() <= 5)
            {
                str_min += value;
            }
        } else {
            if((min_tmp += value).toInt() <= 59)
            {
                str_min += value;
            }
        }
    }
}
bool ParameterConfig::IsFirstRecoilTime()
{
    if(str_category == "control_para" && parameter_name == "first_recoil_time")
    {
        return true;
    }
    return false;
}
bool ParameterConfig::IsNumBetween(float num)
{
    return(num <= high && num >= low);
}
bool ParameterConfig::IsNumNotinSide(float num)
{
    return(num <= high);
}

QString getUsername(QString para)
{
    if(para == "0")
    {
        return "user";
    }
    else if(para == "1"){
        return "admin";
    }
    else if(para == "2"){
        return "dev";
    }
    else {
        return "admin";
    }
}

void ParameterConfig::show_change_result(int result)
{
    if(result != 0)
        result = 0;

    //由权限低的切换成权限高的需要在这里更新参数
    QString str_label = "real_value";
    this->para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, str_label, this->new_set_value);

    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
    this->close();
    deleteLater();
    static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);
}

/*********************************************************
 * @file      : parameter_config.cpp
 * @brief     : left and right button refresh
 * @author    : fxs
 * @date      : 2026-06-10
 * @version   : V1.0
 *********************************************************/

void ParameterConfig::updateBaudrateDisplay()
{
    if (baudrateIndex >=  baudrateList.size()) {
        baudrateIndex = 0;
    }

    if (baudrateIndex < 0) {
        baudrateIndex = baudrateList.size() - 1;
    }
    QString str = modeText;
    str += "  ";
    str += baudrateList.at(baudrateIndex);
    ui->label_input_value->setText(str);
}

/*********************************************************
 * @file      : parameter_config.cpp
 * @brief     : up and down button refresh
 * @author    : fxs
 * @date      : 2026-06-10
 * @version   : V1.0
 *********************************************************/

void ParameterConfig::updateBaudrateDisplayByIndex()
{
    QString str = modeText;

    if (list_range_set_index != 0) {
        PortModeInfo info = para->queryPortModeInfoByIndex(str_category, tab, parameter_name, list_range_set_index);
        baudrateList = info.baudrateList;
        if (baudrateIndex >= baudrateList.size()) {
            baudrateIndex = 0;
        }
    } else {
        baudrateList.clear();
    }

    if (!baudrateList.empty() && list_range_set_index != 0) {
        str += "  ";
        str += baudrateList.at(baudrateIndex);
    }

    ui->label_input_value->setText(str);
}

void ParameterConfig::updateIpDisplay()
{
    QString str;

    for(int i=0;i<4;i++)
    {
        QString seg = ipSegments[i].trimmed();

        if(i == currentIpIndex)
        {
            str += "[" + seg + "]";
        }
        else
        {
            str += seg;
        }

        if(i != 3)
        {
            str += ".";
        }
    }

    ui->label_input_value->setText(str);
}

void ParameterConfig::processIpInput(QString value)
{
    ipInputBuffer += value;

    // ❗防止输入过长异常
    if(ipInputBuffer.length() > 3)
    {
        ipInputBuffer = value;
    }

    bool ok = false;
    int num = ipInputBuffer.toInt(&ok);

    if(!ok)
    {
        ipInputBuffer.clear();

        ui->label_input_value->setStyleSheet("color:red;");
        ui->label_input_value->setText("输入非法");

        return;
    }

    if(num > 255)
    {
        ipInputBuffer.clear();

        ui->label_input_value->setStyleSheet(
            "color:red;"
            "background-color:rgba(255,0,0,30);"
            "border:1px solid red;"
        );

        ui->label_input_value->setText("IP段必须 0~255");

        QTimer::singleShot(1000, this, [this]()
        {
            ui->label_input_value->setStyleSheet("color: rgb(245, 220, 100);");
            updateIpDisplay();
        });

        return;
    }

    if(num >= 0 && num <= 255)
    {
        ipSegments[currentIpIndex] = QString::number(num);
        updateIpDisplay();
    }
}
QString ParameterConfig::getIpValue()
{
    return ipSegments[0] + "." + ipSegments[1] + "." + ipSegments[2] + "." + ipSegments[3];
}

bool ParameterConfig::event(QEvent *event)
{

    QString str_label = "real_value";
    QRegExp regExp("^-?[0-9]+(.[0-9]{1})?$");//一位小数
    QString msg;


    QString category_name = "system";
    QString tab_name = "system_para";

    QStringList range_list;
    QString update_index;
    QString update_item;

    QString usr_from_para;
    QStringList list;//当前值的列表

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_LEFT:
            if(distinguish_flag == 1)
            {
                QString id = this->action_register->action_aditional_list[exact_index].first;
                msg = QString("config -auxiliaryID start ");
                msg += id;
                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                this->comm_acs_thread->cmdManager->lock_ui_off();
            }
        }
        break;
        default:
        if(distinguish_flag == 1)
        {
            this->comm_acs_thread->cmdManager->lock_ui_off();
        }
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            if(this->parameter_type == 1 && send_flag == 0) {
                if (list_range_set_index == 0) {
                    this->comm_acs_thread->cmdManager->lock_ui_off();
                    break;
                }
                if (!baudrateList.empty()) {
                    baudrateIndex++;
                    updateBaudrateDisplay();
                }
            } else if (parameter_type == 2) {
                currentIpIndex++;
                if(currentIpIndex > 3)
                {
                    currentIpIndex = 0;
                }

                ipInputBuffer.clear();
                updateIpDisplay();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            //comm_acs_thread->control_timer->stop();
            if(distinguish_flag == 0)
            {
                if(this->parameter_type == 1 && send_flag == 0) {
                    if (list_range_set_index == 0) {
                        this->comm_acs_thread->cmdManager->lock_ui_off();
                        break;
                    }
                    if (!baudrateList.empty()) {
                        baudrateIndex--;
                        updateBaudrateDisplay();
                    }
                }
                if(parameter_type == 2)
                {
                    currentIpIndex--;
                    if(currentIpIndex < 0)
                    {
                        currentIpIndex = 3;
                    }
                    ipInputBuffer.clear();
                    updateIpDisplay();
                }
                comm_acs_thread->control_timer->stop();
                if((this->parameter_type == 0) && (str_temp.size() > 0)){
                    if(IsFirstRecoilTime())
                    {
                        ui->label_input_value->setText("00:00");
                        str_temp = "";
                        str_hour = "";
                        str_min = "";
                    }
                    else{
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->label_input_value->setText(str_temp);
                    }
                }
                this->comm_acs_thread->cmdManager->lock_ui_off();
            }
            else {
                QString id = this->action_register->action_aditional_list[exact_index].first;
                //msg = QString("config -id ");
                msg = QString("config -auxiliaryID stop ");
                msg += id;
                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                this->comm_acs_thread->cmdManager->lock_ui_off();
            }
            break;

        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->stop();
            if(distinguish_flag == 0)
            {
                //comm_acs_thread->control_timer->stop();
                if(this->parameter_type == 1){
                    if(send_flag == 1)
                    {
                        QStringList mode_list;
                        QStringList list = this->para->query_range_set(this->str_category, this->tab, this->parameter_name,mode_list);
                        //list.size();
                        this->list_range_set_index++;
                        if(this->list_range_set_index >= list.size()){
                            //this->list_range_set_index = list.size() - 1;
                            this->list_range_set_index = 0;
                        }
                        this->new_set_value = QString::number(this->list_range_set_index);
                        ui->label_input_value->setText(list[list_range_set_index]);
                        mode = mode_list[list_range_set_index];
                        //port = new_name;
                    }
                    else
                    {
                        list = this->para->query_range_set(this->str_category, this->tab, this->parameter_name);
                        //list.size();
                        this->list_range_set_index++;
                        if(this->list_range_set_index >= list.size()){
                            this->list_range_set_index = 0;
                        }
                        this->new_set_value = QString::number(this->list_range_set_index);
                        modeText = list[list_range_set_index];
                        updateBaudrateDisplayByIndex();
                    }
                }
            }
            else  //当上下反冲时  up
            {
                if(exact_index == exact_size - 1)
                {
                    exact_index = 0;
                    ui->label_input_value->setText(this->action_register->action_aditional_list[exact_index].second.action_name);
                }
                else
                {
                    ui->label_input_value->setText(this->action_register->action_aditional_list[++exact_index].second.action_name);
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->stop();
            if(distinguish_flag == 0)
            {
                //comm_acs_thread->control_timer->stop();
                if(this->parameter_type == 1){
                    if(send_flag == 1)
                    {
                        QStringList mode_list;
                        QStringList list = this->para->query_range_set(this->str_category, this->tab, this->parameter_name,mode_list);
                        //list.size();
                        list_range_set_index--;
                        if(this->list_range_set_index < 0)
                        {
                            this->list_range_set_index = list.size() - 1;
                        }

                        this->new_set_value = QString::number(this->list_range_set_index);
                        ui->label_input_value->setText(list[list_range_set_index]);
                        mode = mode_list[list_range_set_index];
                    }
                    else
                    {
                        list = this->para->query_range_set(this->str_category, this->tab, this->parameter_name);
                        list_range_set_index--;
                        if(this->list_range_set_index < 0)
                        {
                            this->list_range_set_index = list.size() - 1;
                        }

                        this->new_set_value = QString::number(this->list_range_set_index);
                        modeText = list[list_range_set_index];
                        updateBaudrateDisplayByIndex();
                    }
                }
            }
            else  //当上下反冲时  down
            {
                if(exact_index == 0)
                {
                    exact_index = exact_size - 1;
                    ui->label_input_value->setText(this->action_register->action_aditional_list[exact_index].second.action_name);
                }
                else
                {
                    //ui->label_input_value->setText
                    ui->label_input_value->setText(this->action_register->action_aditional_list[--exact_index].second.action_name);
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            //m++;
            comm_acs_thread->control_timer->stop();
            if(distinguish_flag == 0)
            {
                TLOG_INFO("%d %s %s %d\n", this->ok_step, this->parameter_name.toUtf8().data(), this->str_category.toUtf8().data(), this->parameter_type);
                comm_acs_thread->control_timer->stop();
                QJsonObject obj_value, obj_sub, obj_category, json;
                QString tab_name = this->para->query_tab_name_by_index(str_category, tab);

                if(this->ok_step == 0){
                    if(parameter_type == 2) {
                        QString str_ip = getIpValue();//ip type
                        if(!str_ip.isEmpty()){
                            LOG_DBG(str_ip);
                            bool ip_update = para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, str_label, str_ip);
                            TLOG_INFO("judge parameters.xml");

                            if(this->parameter_name != "current_user")
                            {
                                this->ok_step = 1;//用户更改的步骤由密码是否正确来决定
                            }

                            if (!ip_update) {
                                obj_value[parameter_name] = str_ip;
                                obj_sub[tab_name] = obj_value;
                                obj_category[str_category] = obj_sub;

                                QJsonDocument jsonDoc(obj_category);
                                str_send = jsonDoc.toJson(QJsonDocument::Compact);
                                TLOG_INFO("%s\n", str_send.toUtf8().data());
                                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
                            } else {
                                str_send = "";
                            }

                            ui->label->setText("同步按确认");
                            ui->label_2->setText("不同步按取消");
                            TLOG_INFO("hmi:on ok_step 1");
                        }
                    } else if(this->parameter_type == 0) { //numberic type
                        if(!this->str_temp.isEmpty()) {
                            QString temp =  str_temp;
                            if(temp.at(temp.size() - 1) == '.')
                                str_temp += "0";
                            if(IsNumBetween(str_temp.toFloat())==false && !IsFirstRecoilTime()){
                                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                                this->close();
                                deleteLater();
                                static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);

                            } else {
                                if("first_recoil_time" == this->parameter_name)
                                {
                                    QStringList strlist = str_temp.trimmed().split(':', QString::SkipEmptyParts);
                                    str_temp = "";
                                    str_temp = strlist[0] + "-" + strlist[1];
                                }
                                TLOG_INFO("%s-%d-%s-%s-%s\n", this->str_category.toUtf8().data(), this->tab, this->parameter_name.toUtf8().data(),
                                          str_label.toUtf8().data(), str_temp.toUtf8().data());

                                this->para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, str_label, str_temp);
                                TLOG_INFO("judge parameters.xml");
                                LOG_DBG(parameter_name);
                                if("GroupVolume" == this->parameter_name)
                                {
                                    GroupVolume = str_temp.toInt();
                                }

                                if("GroupVolumeDistance" == this->parameter_name)
                                {
                                    GroupVolumeDistance = str_temp.toInt();
                                    if(GroupVolumeDistance == 0)
                                        GroupVolumeDistance = 1;
                                }

                                if("coal_machine_enter" == this->parameter_name){
                                    g_coal_machine_fence = str_temp.toInt();
                                }

                                if("back_ground_light" == this->parameter_name)
                                {
                                    QString cmd = QString("echo ") + str_temp + QString(" > ") + QString(BACK_GROUND_LIGHT);
                                    system(cmd.toUtf8().data());
                                }

                                if(this->parameter_name == "input_passwd_interval"){
                                    emit this->comm_acs_thread->cmdManager->signal_update_input_passwd_interval(str_temp.toInt());
                                }

                               // str_send = this->str_category +":"+ tab_name + ":" + this->parameter_name + ":" + str_temp;

                                TLOG_INFO("%s\n", str_send.toUtf8().data());
                                if(this->is_global){

                                    obj_value[parameter_name] = str_temp.toInt();
                                    obj_sub[tab_name] = obj_value;
                                    obj_category[str_category] = obj_sub;

                                    QJsonDocument jsonDoc(obj_category);
                                    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));

                                    str_send = jsonDoc.toJson(QJsonDocument::Compact);
                                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());

                                    this->pst->send_para_synchronization_msg(str_send);
                                    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                                    this->close();
                                    deleteLater();
                                    static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);
                                } else if(this->is_global == 0)
                                {
                                    if(this->parameter_name != "current_user")
                                    {
                                        this->ok_step = 1;//用户更改的步骤由密码是否正确来决定
                                    }

                                    obj_value[parameter_name] = str_temp.toInt();
                                    obj_sub[tab_name] = obj_value;
                                    obj_category[str_category] = obj_sub;

                                    QJsonDocument jsonDoc(obj_category);
                                    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));

                                    str_send = jsonDoc.toJson(QJsonDocument::Compact);
                                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());

                                    ui->label->setText("同步按确认");
                                    ui->label_2->setText("不同步按取消");
                                    TLOG_INFO("hmi:on ok_step 1");
                                }
                            }
                        }else{
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            deleteLater();
                        }
                    } else { //switch type
                        QByteArray ba;
                        ba.append(new_set_value);
                        const char* c = ba.data();
                        new_set_value = QString(c);

                        if(this->str_category == "healthy" && parameter_name == "human_lock_switch")//人员闭锁开关
                        {
                            QString msg;
                            if(this->new_set_value == "0")
                            {
                                msg = "config -hlock off";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                            else if(this->new_set_value == "1")
                            {
                                msg = "config -hlock on";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                        }
                        else if((this->str_category == "healthy") && ("user_management" == this->parameter_name))
                        {
                            if(this->new_set_value == "0")
                            {
                                syzk_user = "admin";
                            } else {
                                QString category_name = "system";
                                QString tab_name = "system_para";
                                QString para_name = "current_user";
                                QString label = "real_value";
                                usr_from_para = this->para->query(category_name, tab_name, para_name, label);
                                syzk_user = getUsername(usr_from_para);
                            }
                        }
                        else if(this->str_category == "healthy" && parameter_name == "infrared_pyroelectric_switch")//红外开关
                        {
                            QString msg;
                            if(this->new_set_value == "0")
                            {
                                msg = "config -infrared off";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                            else if(this->new_set_value == "1")
                            {
                                msg = "config -infrared on";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                        }
                        else if(this->str_category == "control_para" && parameter_name == "timing")//自动反冲洗--自动定时
                        {
                            QString msg;
                            if(this->new_set_value == "0")
                            {
                                msg = "config -autobackwash off";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                            else if(this->new_set_value == "1")
                            {
                                msg = "config -autobackwash on";
                                this->comm_acs_thread->send_socket_msg_to_acs(msg);
                            }
                        }

                        if("current_user" ==  this->parameter_name)
                        {
                            TLOG_INFO("ui->label_input_value %s\n", ui->label_input_value->text().toUtf8().data());
                            if(((syzk_user == "dev") && (ui->label_input_value->text() == "admin")) || ((syzk_user == "user") && (ui->label_input_value->text() == "admin")) || ((syzk_user == "user") && (ui->label_input_value->text() == "dev")))
                            {
                                Dialog_UserCheckAndLogin *p_window = new Dialog_UserCheckAndLogin(1,ui->label_input_value->text(),&this->switch_user_flag);
                                connect(p_window,SIGNAL(change_result(int)),this,SLOT(show_change_result(int)));//修改用户权限不需要同步
                                p_window->show();
                                this->widget_stack->push(this);
                                this->comm_acs_thread->cmdManager->window = p_window;
                            }
                            else {
                                syzk_user = ui->label_input_value->text();
                                QString menu = "accessManagemnet";
                                this->para->query_for_menuconfig(menu,menuconfig_list);
                                switch_user_flag = true;
                            }
                        }

                        if("video_frames" == this->parameter_name)
                        {
                            range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
                            update_index = this->para->query(category_name, tab_name, parameter_name, str_label);
                            update_item = range_list[update_index.toInt()];
                            video_frames = update_item.toInt();
                        }

                        if("current_user" !=  this->parameter_name)
                        {
                            qDebug() << "nainai++";
                            setValue = this->para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, str_label, this->new_set_value);
                            if (!baudrateList.empty() && baudrateIndex < baudrateList.size()) {
                                QString baudrate = "baudrate_value";
                                QString baudrate_index = QString::number(baudrateIndex);
                                setBaudrate = para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, baudrate, baudrate_index);
                            }
                        }
                        if(!parameter_name.compare("hydraulic_supporter_type"))
                        {
                            ActionRegister::getInstance()->reload();
                            emit this->comm_acs_thread->cmdManager->signal_update_sensor_data(mainPage::View_SUP_TYPE);
                            QJsonObject json;
                            json["reload"] = "action.xml";
                            QJsonDocument jsonDoc(json);
                            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());

                        }

                        if(this->is_global){
                            if("current_user" !=  this->parameter_name)//更改用户不能一次确定 第一次什么都不做等结果出来
                            {
                                LOG_DBG("111");
                                TLOG_INFO("INFO %s %d", this->str_category.toStdString().c_str(), this->tab);
                                obj_value[parameter_name] = new_set_value.toInt();
                                obj_sub[tab_name] = obj_value;
                                obj_category[str_category] = obj_sub;

                                QJsonDocument jsonDoc(obj_category);
                                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                                str_send = jsonDoc.toJson(QJsonDocument::Compact);
                                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());

                                this->pst->send_para_synchronization_msg(str_send);
                                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                                this->close();
                                deleteLater();
                                if(g_port_visualization == 0)
                                    static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);
                                else {
                                    static_cast<port_visualization *>(this->comm_acs_thread->cmdManager->window)->update_label(index_ui);
                                }
                            }//emit change_user();
                        }else if(this->is_global == 0)
                        {
                            this->ok_step = 1;
                            if("current_user" !=  this->parameter_name)
                            {
                                if(this->str_category == "healthy" && tab_name == "port_config")
                                {
                                    str_send = "";
                                    QJsonObject port_value;
                                    if (!setValue || !setBaudrate) {
                                        update_index = this->para->query(str_category, tab_name, parameter_name, str_label);
                                        update_item = this->para->query_element_name(str_category, tab_name, parameter_name, update_index.toInt());
//                                        QJsonObject json_value, json_sub, json_category;
                                        if (setValue == 0) obj_value["mode"] = update_item;
                                        if (!baudrateList.empty() && baudrateIndex < baudrateList.size() && setBaudrate == 0) {
                                            obj_value["baudrate"] = baudrateList[baudrateIndex].toInt();
                                        }

                                        port_value[parameter_name] = obj_value;
                                        obj_sub[tab_name] = port_value;
                                        obj_category[str_category] = obj_sub;

                                        QJsonDocument jsonDoc(obj_category);
                                        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                                        str_send = jsonDoc.toJson(QJsonDocument::Compact);
                                        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
                                    }
                                } else {
                                    obj_value[parameter_name] = new_set_value.toInt();
                                    obj_sub[tab_name] = obj_value;
                                    obj_category[str_category] = obj_sub;

                                    QJsonDocument jsonDoc(obj_category);
                                    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
                                }
                            }
                            if("current_user" !=  this->parameter_name)
                            {
                                ui->label->setText("同步按确认");
                                ui->label_2->setText("不同步按取消");
                                TLOG_INFO("hmi:on ok_step 1");
                            } else {//管理员切换开发人员/操作员，开发人员切换操作员，需要在此处更新参数
                                this->para->update("parameters.xml", this->str_category, this->tab, this->parameter_name, str_label, this->new_set_value);
                            }
                        }
                    }
                } else if (this->ok_step == 1){
                    TLOG_INFO("INFO2 %s--%d--%s", this->str_category.toStdString().c_str(), this->tab, this->parameter_name.toStdString().c_str());

                    if((this->str_category == "healthy") && (this->tab == 1))
                    {
                        this->pst->send_para_synchronization_msg(str_send);
                    }
                    else
                    {
                        if("current_user" != this->parameter_name && str_send != "")//用户切换供调试使用，不需要同步参数
                        {
                            this->pst->send_para_synchronization_msg(str_send);
                        }
                    }

                    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                    this->close();
                    deleteLater();
                    static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            if(distinguish_flag == 0)
            {
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                if(this->ok_step == 1){
                    static_cast<leftPane *>(this->comm_acs_thread->cmdManager->window)->load_parameters(tab,index_ui,switch_user_flag);
                }
                deleteLater();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_0:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->parameter_type == 0){
                    QString mytemp = str_temp;

                    //if((mytemp == ".")|| (str_temp.indexOf(".") >= 0))
                    if(IsFirstRecoilTime())//首次反冲洗时间设置
                    {
                        FirstRecoilTimeProc("0");
                        str_temp = str_hour + ":" + str_min;
                        ui->label_input_value->setText(str_temp);
                    }
                    else{
                        if(IsNumNotinSide((mytemp += "0").toFloat()))
                        {
                            if(mytemp == "00")
                            {

                            } else {
                                if((regExp.exactMatch(mytemp)) == 1)
                                {
                                    str_temp += "0";
                                }
                            }
                        }
                        ui->label_input_value->setText(str_temp);
                    }
                } else if (parameter_type == 2) {
                    processIpInput("0");
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_1:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("1");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("1");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_2:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("2");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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
                    } else if (parameter_type == 2) {
                        processIpInput("2");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_3:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("3");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("3");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_4:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("4");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("4");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_5:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("5");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("5");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_6:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("6");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("6");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_7:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("7");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("7");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_PUSH_PULL:
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("8");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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
                    } else if (parameter_type == 2) {
                        processIpInput("8");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_9:
             if(distinguish_flag == 0)
             {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;

                        if(IsFirstRecoilTime())//首次反冲洗时间设置
                        {
                            FirstRecoilTimeProc("9");
                            str_temp = str_hour + ":" + str_min;
                            ui->label_input_value->setText(str_temp);
                        }
                        else{
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

                    } else if (parameter_type == 2) {
                        processIpInput("9");
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F1: //used as "."
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0){
                        QString mytemp = str_temp;
                        mytemp += ".";
                        if(this->is_number == 0)
                        {
                            if((mytemp == "-.") || (mytemp == ".") || (str_temp.indexOf(".") >= 0))
                            {

                            }
                            else
                            {
                                str_temp += ".";
                            }
                        }
                        else {

                        }
                        ui->label_input_value->setText(str_temp);
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F2: //used as "-"
            if(distinguish_flag == 0)
            {
                comm_acs_thread->control_timer->stop();
                if(this->ok_step == 0)
                {
                    if(this->parameter_type == 0)
                    {
                        str_temp.clear();
                        str_temp += "-";
                        ui->label_input_value->setText(str_temp);

                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;
    }
    return QWidget::event(event);
}

