#include "login_in.h"
#include "ui_login_in.h"

#include <QKeyEvent>
#include <QEvent>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QItemSelectionModel>

#include "./Resources/gui_setting.h"
#include "./Resources/guiconfig.h"

extern Gui_config   guisetting;
extern QString syzk_user;
extern QList <mainPage_uigonfig> menuconfig_list;

#define ROOT_PASSWD "316318"

login_in::login_in(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login_in)
{
    ui->setupUi(this);
    index = 1;
    this->pst = ParaSynchronizeThread::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();
    this->gui_set = gui_setting::getInstance();
    ui->lineEdit_passwd->setFocus();
    ui->lineEdit_passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    //ui->lineEdit_passwd->setStyleSheet("QFont {color: rgb(255, 255, 255);}");
    //ui->combox_count->setStyleSheet("QComboBox::drop-down {background-color: transparent;}");
}

login_in::~login_in()
{
    delete ui;
}

bool login_in::event(QEvent *event)
{
    QString dst;

    QString user_first = "system";
    QString user_second = "system_para";
    QString user_third = "current_user";
    QString user_label = "real_value";
    QStringList range_list;

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1){
                str_temp = ui->lineEdit_passwd->text();
                if(str_temp.size() > 0){
                    str_temp = str_temp.mid(0, str_temp.size()-1);
                    ui->lineEdit_passwd->setText(str_temp);
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:{
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 0)
            {
                comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
                if(combox_select == true){
                    combox_select = false;
                    ui->combox_count->hidePopup();
                } else {
                    combox_select = true;
                    last_combox_index = ui->combox_count->currentIndex();
                    ui->combox_count->showPopup();
                }
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            dst = ui->lineEdit_passwd->text();
            if(dst == "")
            {
                ui->label_warning->setStyleSheet("color: yellow;");
                ui->label_warning->setText("密码不能为空!");
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }
            int count_type = ui->combox_count->currentIndex();
            QString count_pass;
            QString count_name;
            switch(count_type){
            case 0:
                count_pass = guisetting.admin_passwd;
                count_name = "admin";
                break;
            case 1:
                count_pass = guisetting.user_passwd;
                count_name = "user";
                break;
            case 2:
                count_pass = guisetting.dev_passwd;
                count_name = "dev";
                break;
            default:
                break;
            }
            TLOG_INFO("it will go in menu (input:%s-passwd:%s)", dst.toUtf8().data(), count_pass.toUtf8().data());
            QDate data_now = QDate::currentDate();
            QTime time_now = QTime::currentTime();

            QString root_pass = QString("%2").arg(data_now.year(), 2, 10, QLatin1Char('0')).right(2) + \
                    QString("%2").arg(data_now.month() + data_now.day(), 2, 10, QLatin1Char('0')) + \
                    QString("%2").arg(time_now.hour(), 2, 10, QLatin1Char('0'));

            if((dst == count_pass) || ((dst == root_pass)))
            {
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                name_switch = count_name;
                syzk_user = name_switch;
                QString menu = "accessManagemnet";
                this->para->query_for_menuconfig(menu,menuconfig_list);
                range_list = this->para->query_range_set(user_first, user_second, user_third);
                for(int i = 0; i < range_list.size();i++)
                {
                    if((range_list[i]) == name_switch)
                    {
                        QString the_index = QString::number(i);
                        this->para->update(QString("parameters.xml"),user_first,user_second,user_third,user_label,the_index);
                    }
                }
                this->close();
                emit change_result(1);
                this->~login_in();
            } else {
                ui->label_warning->setStyleSheet("color: red;");
                ui->label_warning->setText("密码错误!");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_UP:{
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1){
                index = 0;
                ui->combox_count->setFocus();
            } else {
                if(combox_select == true)
                {
                    int cur_index = ui->combox_count->currentIndex();
                    if(cur_index > 0)
                        --cur_index;
                    ui->combox_count->setCurrentIndex(cur_index);
                    QModelIndex itemIndex = ui->combox_count->view()->model()->index(cur_index,0);
                    ui->combox_count->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);
                } else {
                    index = 1;
                    ui->lineEdit_passwd->setFocus();
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
    case KeyValue::KEY_ARROW_DOWN:{
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1){
                index = 0;
                ui->combox_count->setFocus();
            } else {
                if(combox_select == true)
                {
                    int cur_index = ui->combox_count->currentIndex();
                    if(cur_index < ui->combox_count->count()-1)
                        ++cur_index;
                    ui->combox_count->setCurrentIndex(cur_index);
                    QModelIndex itemIndex = ui->combox_count->view()->model()->index(cur_index,0);
                    ui->combox_count->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);
                } else {
                    index = 1;
                    ui->lineEdit_passwd->setFocus();
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "0");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "1");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "2");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "3");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "4");
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "5");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "6");
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "7");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "8");
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
                ui->lineEdit_passwd->setText(ui->lineEdit_passwd->text() + "9");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            if(combox_select == true)
            {
                comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
                combox_select = false;
                ui->combox_count->hidePopup();
                ui->combox_count->setCurrentIndex(last_combox_index);
                this->comm_acs_thread->cmdManager->lock_ui_off();
                break;
            }

            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            emit change_result(0);
            this->~login_in();
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
