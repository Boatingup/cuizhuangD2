#include "dialog_timer_setup.h"
#include "ui_dialog_timer_setup.h"
#include <QEvent>
#include <QKeyEvent>
#include <QFile>
#include <unistd.h>
#include <sys/reboot.h>

Dialog_timer_setup::Dialog_timer_setup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_timer_setup)
{
    ui->setupUi(this);

    QFile file(":/qss/set_time.qss");
    if(!file.open(QIODevice::ReadOnly)){
        //LOG(INFO) << "open qss file failed in " << __FILE__ << __LINE__;
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();      
    }

    comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->pst = ParaSynchronizeThread::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    datetime = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(datetime);
    ui->dateTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    init_sections();
    ui->dateTimeEdit->setSelectedSection((QDateTimeEdit::Section)Sections[section_index]);
    ui->label->setText("请设置时间");
}

void Dialog_timer_setup::init_sections()
{
    Sections.push_back(QDateTimeEdit::YearSection);
    Sections.push_back(QDateTimeEdit::MonthSection);
    Sections.push_back(QDateTimeEdit::DaySection);
    Sections.push_back(QDateTimeEdit::HourSection);
    Sections.push_back(QDateTimeEdit::MinuteSection);
    Sections.push_back(QDateTimeEdit::SecondSection);
}

Dialog_timer_setup::~Dialog_timer_setup()
{
    delete ui;
}



bool Dialog_timer_setup::event(QEvent *event)
{

    QString setTime;//时间同步命令
    QString str_send;//工作面同步命令
    QString setTime_format;//时间同步命令

    // for test  ---start
    QStringList time_list;
    QString TimeYear;
    QString TimeMonth;
    QString TimeDay;
    QString TimeHour;
    QString TimeMinute;
    QString TimeSec;
    QString syntime;
    QString cmd;
    // for test  ---end
    //setTime+= datetime.toString("yyyy-MM-dd HH:mm:ss");
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if((section_index < 5) && (section_index >= 0))
            {
                ui->dateTimeEdit->setSelectedSection((QDateTimeEdit::Section)Sections[++section_index]);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //选择section
            break;
        case KeyValue::KEY_ARROW_LEFT:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(section_index > 0 && section_index <= 5)
            {
                ui->dateTimeEdit->setSelectedSection((QDateTimeEdit::Section)Sections[--section_index]);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //选择section
            break;

        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(Sections[section_index] == QDateTimeEdit::YearSection)
            {
                datetime_real = datetime.addYears(1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::MonthSection)
            {
                datetime_real = datetime.addMonths(1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::DaySection)
            {
                datetime_real = datetime.addDays(1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::HourSection)
            {
                datetime_real = datetime.addSecs(3600);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if  (Sections[section_index] == QDateTimeEdit::MinuteSection)
            {
                datetime_real = datetime.addSecs(60);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if  (Sections[section_index] == QDateTimeEdit::SecondSection)
            {
                datetime_real = datetime.addSecs(1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else
            {

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //处理数据增加减少
            break;
        case KeyValue::KEY_ARROW_DOWN:
            //comm_acs_thread->control_timer->stop();
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(Sections[section_index] == QDateTimeEdit::YearSection)
            {
                datetime_real = datetime.addYears(-1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::MonthSection)
            {
                datetime_real = datetime.addMonths(-1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::DaySection)
            {
                datetime_real = datetime.addDays(-1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if (Sections[section_index] == QDateTimeEdit::HourSection)
            {
                datetime_real = datetime.addSecs(-3600);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if  (Sections[section_index] == QDateTimeEdit::MinuteSection)
            {
                datetime_real = datetime.addSecs(-60);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else if  (Sections[section_index] == QDateTimeEdit::SecondSection)
            {
                datetime_real = datetime.addSecs(-1);
                ui->dateTimeEdit->setDateTime(datetime_real);
                datetime = datetime_real;
            }
            else
            {

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //处理数据增加减少
            break;

        case KeyValue::KEY_OK:
        {
            comm_acs_thread->control_timer->stop();
            setTime = "date -s ";
            setTime+="\"";
            setTime+= datetime.toString("yyyy-MM-dd HH:mm:ss");
            setTime+="\"";
            setTime_format = datetime.toString("yyyy-MM-dd-HH-mm-ss");
            system(setTime.toStdString().c_str()); // 0312 表示3月12日，1025表示当前时分
            system("/sbin/hwclock -w"); // 同步系统时间
            sync();
            ui->label->setText("时间设置成功");

            QJsonObject param_obj;
            param_obj["timesyn"] = setTime_format; // 直接放键值对

            QJsonDocument doc(param_obj);
            str_send = doc.toJson(QJsonDocument::Compact);

            TLOG_INFO("send time syn %s", str_send.toStdString().c_str());

            this->pst->send_para_synchronization_msg(str_send);
            TLOG_INFO("send time syn finished %s", str_send.toStdString().c_str());
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_CANCEL:
            //关闭窗口
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~Dialog_timer_setup();
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
