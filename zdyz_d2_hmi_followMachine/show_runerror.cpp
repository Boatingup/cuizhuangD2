#include "show_runerror.h"
#include "ui_show_runerror.h"
//#include "glog/logging.h"
#include <QEvent>
#include <QKeyEvent>
int show_runerror::GetStylePath(QString &css)
{
    QString str_path = QCoreApplication::applicationDirPath();
    QString Current_Path = str_path;
    str_path += "/style/qss/style.qss";
    //path = str_path;
    QFile file(str_path);
    int m = file.open(QFile::ReadOnly);
    if (m == true)
    {
        css = file.readAll(); //获取qss中全部字符
        return 0;
        //this->setStyleSheet(str); //设置样式表
    }
    else
    {
        return 1;
    }
}

show_runerror::show_runerror(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::show_runerror)
{
    ui->setupUi(this);
    QString category_name = "set_para";
    //int tab = 0;
    QString tab_name = "System_error";
    QString parameter_name = "Acs_error";
    this->para = Parameters::getInstance();

    range_list = this->para->query_error_set(category_name, tab_name, parameter_name, map_error);
    DBG_INFO_OLD("size :%d\n", range_list.size());

    /*
    QString css;
    int rtn = GetStylePath(css);
    if(rtn == 0)
    {
        this->setStyleSheet(css); //设置样式表
    }
    else
    {
        LOG(INFO) << "load css failed";
    }
    */

    QFile file(":/qss/run_error.qss");
    if(!file.open(QIODevice::ReadOnly)){
        //LOG(INFO) << "open qss file failed in " << __FILE__ << __LINE__;
    }else{
        QString ss = file.readAll();
        LOG(INFO) << ss.toLocal8Bit().data();
        this->setStyleSheet(ss);
        file.close();
    }


    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_system_fault(QString,QString)),
            this,
            SLOT(update_system_fault(QString,QString)));
    this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    /* 打印后台接收错误 */
    if(this->comm_acs_thread->cmdManager->last_msg_error.size() != 0)
    {
        for(int i = 0; i < this->comm_acs_thread->cmdManager->last_msg_error.size();i++)
        {
            show_error(this->comm_acs_thread->cmdManager->last_msg_error[i].current_time,this->comm_acs_thread->cmdManager->last_msg_error[i].msg_error);
        }
    }

    ui->textEdit_sys_errors->moveCursor(QTextCursor::End);

    LOG(INFO) << "end----time-----press key";
}

void show_runerror::show_error(QString time,QString str)
{
    LOG(INFO) << str.toStdString().c_str();
    if (map_error.contains(str))
    {
       int index = map_error.value(str);
       ui->textEdit_sys_errors->append(time);
       ui->textEdit_sys_errors->append(range_list[index]);
    }
    else {
        LOG(INFO) << "error not found !";
    }
}

void show_runerror::update_system_fault(QString time,QString info)
{
     //int index = map_error   
     show_error(time,info);
     this->comm_acs_thread->cmdManager->lock_ui_off();
     //ui->textEdit_sys_errors->append(range_list[index]);

     /*
    if(str == "left_can_error")
    {
        ui->textEdit_sys_errors->append("左邻架can总线故障");
    }else if(str == "right_can_error")
    {
        ui->textEdit_sys_errors->append("右邻架can总线故障");
    }else if(str == "left_net_error")
    {
        ui->textEdit_sys_errors->append("左邻架网络故障");

    }else if(str == "right_net_error")
    {
        ui->textEdit_sys_errors->append("右邻架网络故障");
    }else if(str == "connect_uart_error")
    {
        ui->textEdit_sys_errors->append("连接器板串口故障");
    }else if(str == "key_uart_error")
    {
        ui->textEdit_sys_errors->append("按键板串口故障");
    }
    else if(str == "sensor_pressure1_error")
    {
        ui->textEdit_sys_errors->append("压力传感器一故障");
    }
    else if(str == "sensor_pressure2_error")
    {
        ui->textEdit_sys_errors->append("压力传感器二故障");
    }
    else if(str == "sensor_pressure3_error")
    {
        ui->textEdit_sys_errors->append("压力传感器三故障");
    }
    else if(str == "sensor_pressure4_error")
    {
        ui->textEdit_sys_errors->append("压力传感器四故障");
    }
    else if(str == "sensor_pressure5_error")
    {
        ui->textEdit_sys_errors->append("压力传感器五故障");
    }
    else if(str == "sensor_angle1_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器一故障");
    }
    else if(str == "sensor_angle2_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器二故障");
    }
    else if(str == "sensor_angle3_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器三故障");
    }
    else if(str == "sensor_angle4_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器四故障");
    }
    else if(str == "sensor_angle5_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器五故障");
    }
    else if(str == "sensor_angle6_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器六故障");
    }
    else if(str == "sensor_infra_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器六故障");
    }
    else if(str == "sensor_trip_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器六故障");
    }
    else if(str == "connect_driver_error")
    {
        ui->textEdit_sys_errors->append("倾角传感器六故障");
    }
    else if(str == "connect_up_backflush_error")
    {
        ui->textEdit_sys_errors->append("上反冲故障");
    }
    else if(str == "connect_down_backflush_error")
    {
        ui->textEdit_sys_errors->append("下反冲故障");
    }
    else if(str == "connect_rjgt_error")
    {
        ui->textEdit_sys_errors->append("rjgt故障");
    }
    else if(str == "connect_sht30_error")
    {
        ui->textEdit_sys_errors->append("sht30故障");
    }
    else if(str == "connect_mpu6050_error")
    {
        ui->textEdit_sys_errors->append("mpu6050故障");
    }
    */
}


bool show_runerror::event(QEvent *event)
{
    QKeyEvent *key_event = (QKeyEvent *)(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            ui->textEdit_sys_errors->moveCursor(QTextCursor::Down);
            ui->textEdit_sys_errors->moveCursor(QTextCursor::Down);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            ui->textEdit_sys_errors->moveCursor(QTextCursor::Up);
            ui->textEdit_sys_errors->moveCursor(QTextCursor::Up);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~show_runerror();
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

show_runerror::~show_runerror()
{
    delete ui;
}
