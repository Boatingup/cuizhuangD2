#include "dialog_usercheckandlogin.h"
#include "ui_dialog_usercheckandlogin.h"

#include <QKeyEvent>
#include <QEvent>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QComboBox>

#include "./Resources/gui_setting.h"
#include "./Resources/guiconfig.h"

extern Gui_config   guisetting;
extern QString syzk_user;
extern gui_setting gui_set;
extern QList <mainPage_uigonfig> menuconfig_list;

#define ROOT_PASSWD "316318"

Dialog_UserCheckAndLogin::Dialog_UserCheckAndLogin(int flag, QString switch_name, bool * switch_flag, QWidget *parent) :
    QDialog(parent),ui(new Ui::Dialog_UserCheckAndLogin),verify_flag(flag),p_switch_flag(switch_flag)
{
    ui->setupUi(this);
    QFile file(":/qss/usercheckandlogin.qss");
    if(!file.open(QIODevice::ReadOnly)){
        //LOG(INFO) << "open qss file failed in " << __FILE__ << __LINE__;
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }

    index = 0;
    this->pst = ParaSynchronizeThread::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();
    this->gui_set = gui_setting::getInstance();
    ui->lineEdit_oldpasswd->setFocus();
    name_switch = switch_name;
    ui->label_msg->setText(" ");
    ui->label_title->hide();
    ui->label_input->hide();
    ui->lineEdit_input->hide();
    ui->label_count->hide();
    ui->combox_count->hide();

    if(flag == 2)
    {
        ui->label_title->show();
        ui->label_usr->hide();
        ui->label_info->hide();
        ui->lineEdit_newpasswd->setEnabled(false);
        ui->lineEdit_newpasswd->hide();
        ui->label_3->hide();
        ui->label_title->hide();
        ui->label_2->hide();
        ui->lineEdit_oldpasswd->hide();
        ui->label_old_passwd->hide();
        ui->lineEdit_old_passwd->hide();
        ui->label_input->show();
        ui->lineEdit_input->show();
        ui->label_count->show();
        ui->combox_count->show();

        ui->label_count->setGeometry(130, 75, 50, 50);
        ui->label_count->setPixmap(QPixmap(QString::fromUtf8(":/new/menu_page/menu_item/account.png")));
        ui->combox_count->setGeometry(200, 80, 180, 40);
        ui->combox_count->setStyleSheet("background-color: white;");

        ui->label_input->setGeometry(130, 145, 50, 50);
        ui->label_input->setPixmap(QPixmap(QString::fromUtf8(":/new/menu_page/menu_item/passwd.png")));
        ui->lineEdit_input->setGeometry(200, 152, 180, 20);
        ui->lineEdit_input->setEchoMode(QLineEdit::Password);
        ui->lineEdit_input->setPlaceholderText("请输入密码");

        index = 255;
    }
    else
    {
        ui->label_usr->hide();
        ui->label_info->setText("切换到:");
        ui->label_title->show();

        if(switch_name == "user")
        {
            ui->label_title->setText("操作员");
            ui->label_2->setText("操作员密码:");

        } else if (switch_name == "admin") {
            ui->label_title->setText("管理员");
            ui->label_2->setText("管理员密码:");

        } else if (switch_name == "dev") {
            ui->label_title->setText("开发");
            ui->label_2->setText("开发人员密码:");
        }

        ui->lineEdit_oldpasswd->setEchoMode(QLineEdit::Password);
        ui->lineEdit_oldpasswd->setPlaceholderText("请输入原密码");
        ui->lineEdit_newpasswd->setEnabled(false);
        ui->lineEdit_newpasswd->hide();
        ui->label_3->hide();
        ui->label_old_passwd->hide();
        ui->lineEdit_old_passwd->hide();
    }
}

bool Dialog_UserCheckAndLogin::event(QEvent *event)
{
    QString first = "user_passwd";
    QString second_user = "user";
    QString second_admin = "admin";
    QString second_dev = "dev";
    QString content_encrpt;
    QString dst, src, sure;

    QString user_first = "system";
    QString user_second = "system_para";
    QString user_third = "current_user";
    QString user_label = "real_value";
    QStringList range_list;

    QString str_send = "sync_passwd:";

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                {
                    str_temp = ui->lineEdit_newpasswd->text();
                    if(str_temp.size() > 0){
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->lineEdit_newpasswd->setText(str_temp);
                    }

                } else if(index == 1) {
                    str_temp = ui->lineEdit_oldpasswd->text();
                    if(str_temp.size() > 0){
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->lineEdit_oldpasswd->setText(str_temp);
                    }
                } else if(index == 0) {
                    str_temp = ui->lineEdit_old_passwd->text();
                    if(str_temp.size() > 0){
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->lineEdit_old_passwd->setText(str_temp);
                    }
                }
            }
            else {
                if(index == 255){
                    str_temp = ui->lineEdit_input->text();
                    if(str_temp.size() > 0){
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->lineEdit_input->setText(str_temp);
                    }
                } else {
                    str_temp = ui->lineEdit_oldpasswd->text();
                    if(str_temp.size() > 0){
                        str_temp = str_temp.mid(0, str_temp.size()-1);
                        ui->lineEdit_oldpasswd->setText(str_temp);
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)//修改密码
            {
                dst = ui->lineEdit_oldpasswd->text();
                src = ui->lineEdit_old_passwd->text();
                sure = ui->lineEdit_newpasswd->text();
                ui->label_msg->setStyleSheet("color:yellow;");
                if (src == "") {
                    ui->label_msg->setText("原密码不能为空!");
                } else if(dst == "") {
                    ui->label_msg->setText("新密码不能为空!");
                } else if (sure == "") {
                    ui->label_msg->setText("确认密码不能为空!");
                }

                else
                {
                    if(syzk_user == "user")
                    {
                        if(ui->lineEdit_old_passwd->text() != guisetting.user_passwd)
                        {
                            ui->label_msg->setText("密码错误!");
                        } else if (dst != sure) {
                            ui->label_msg->setText("密码输入不一致!");
                        }else {
                            this->para->Encrypt(dst, content_encrpt);
                            guisetting.user_passwd = ui->lineEdit_newpasswd->text();
                            gui_set->write(first, second_user, content_encrpt);
                            str_send += first + ":" + second_user + ":" + content_encrpt;
                            this->pst->send_para_synchronization_msg(str_send);
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            this->~Dialog_UserCheckAndLogin();
                        }
                    } else if(syzk_user == "admin") {
                        if(ui->lineEdit_old_passwd->text() != guisetting.admin_passwd)
                        {
                            ui->label_msg->setText("密码错误!");
                        } else if (dst != sure) {
                            ui->label_msg->setText("密码输入不一致!");
                        } else {
                            this->para->Encrypt(dst, content_encrpt);
                            gui_set->write(first, second_admin, content_encrpt);
                            str_send += first + ":" + second_admin + ":" + content_encrpt;
                            this->pst->send_para_synchronization_msg(str_send);
                            guisetting.admin_passwd = ui->lineEdit_newpasswd->text();
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            this->~Dialog_UserCheckAndLogin();
                        }
                    } else if(syzk_user == "dev") {
                        if(ui->lineEdit_old_passwd->text() == guisetting.dev_passwd)
                        {
                            ui->label_msg->setText("密码错误!");
                        } else if (dst != sure) {
                            ui->label_msg->setText("密码输入不一致!");
                        }else {
                            guisetting.dev_passwd = ui->lineEdit_newpasswd->text();
                            this->para->Encrypt(dst, content_encrpt);
                            gui_set->write(first, second_dev, content_encrpt);
                            str_send += first + ":" + second_dev + ":" + content_encrpt;
                            this->pst->send_para_synchronization_msg(str_send);
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            this->~Dialog_UserCheckAndLogin();
                        }
                    }
                }
            }
            else if(verify_flag == 2)
            {
                if(index == 254)
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
                dst = ui->lineEdit_input->text();
                if(dst == "")
                {
                    ui->label_warning->setGeometry(190, 20, 150, 20);
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
                if((dst == count_pass) || ((dst == ROOT_PASSWD)))
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
                            qDebug() << "++++xxxx++++";
                            this->para->update(QString("parameters.xml"),user_first,user_second,user_third,user_label,the_index);
                        }
                    }
                    this->close();
                    emit change_result(1);
                    this->~Dialog_UserCheckAndLogin();
                    (*p_switch_flag)= true;
                } else {
                    ui->label_warning->setGeometry(220, 20, 150, 20);
                    ui->label_warning->setStyleSheet("color: red;");
                    ui->label_warning->setText("密码错误!");
                    (*p_switch_flag)= false;
                }
            }
            else //切换用户
            {
                dst = ui->lineEdit_oldpasswd->text();
                if(dst == "")
                {
                    ui->label_msg->setText("密码不能为空");
                }
                else
                {
                    TLOG_INFO("switch:%s; cur_user:%s; input:%s", name_switch.toUtf8().data(), syzk_user.toUtf8().data(), dst.toUtf8().data());
                    if((name_switch == "admin") && (syzk_user == "dev"))
                    {
                        if((dst == guisetting.admin_passwd)||(dst == ROOT_PASSWD))
                        {
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
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            emit change_result(1);
                            this->~Dialog_UserCheckAndLogin();
                            (*p_switch_flag)= true;
                        } else {
                            ui->label_msg->setText("密码错误");
                            (*p_switch_flag)= false;
                        }
                    } else if((name_switch == "admin") && (syzk_user == "user")) {
                        if((dst == guisetting.admin_passwd)||(dst == ROOT_PASSWD))
                        {
                            syzk_user = name_switch;
                            range_list = this->para->query_range_set(user_first, user_second, user_third);
                            for(int i = 0; i < range_list.size();i++)
                            {
                                if((range_list[i]) == name_switch)
                                {
                                    QString the_index = QString::number(i);
                                    this->para->update(QString("parameters.xml"),user_first,user_second,user_third,user_label,the_index);
                                }
                            }
                            QString menu = "accessManagemnet";
                            this->para->query_for_menuconfig(menu,menuconfig_list);
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            emit change_result(1);
                            this->~Dialog_UserCheckAndLogin();
                            (*p_switch_flag)= true;
                        } else {
                            ui->label_msg->setText("密码错误");
                            (*p_switch_flag) =false;
                        }
                    } else if((name_switch == "dev") && (syzk_user == "user")) {
                        if((dst == guisetting.dev_passwd)||(dst == ROOT_PASSWD))
                        {
                            syzk_user = name_switch;
                            range_list = this->para->query_range_set(user_first, user_second, user_third);
                            for(int i = 0; i < range_list.size();i++)
                            {
                                if((range_list[i]) == name_switch)
                                {
                                    QString the_index = QString::number(i);
                                    this->para->update(QString("parameters.xml"),user_first,user_second,user_third,user_label,the_index);
                                }
                            }
                            QString menu = "accessManagemnet";
                            this->para->query_for_menuconfig(menu,menuconfig_list);
                            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                            this->close();
                            emit change_result(1);
                            this->~Dialog_UserCheckAndLogin();
                            (*p_switch_flag)= true;
                        } else {
                            ui->label_msg->setText("密码错误");
                            (*p_switch_flag) = false;
                        }
                    } else {
                        TLOG_INFO("user role error");
                        this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                        this->close();
                        this->~Dialog_UserCheckAndLogin();
                        (*p_switch_flag) = false;
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP://textedit
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0) {
                if(index == 1)
                {
                    index = 0;
                    ui->lineEdit_old_passwd->setFocus();
                } else if (index == 2){
                    index = 1;
                    ui->lineEdit_oldpasswd->setFocus();
                } else if (index == 0){
                    index = 2;
                    ui->lineEdit_newpasswd->setFocus();
                }
            } else if (verify_flag == 2){
                if(index == 255){
                    index = 254;
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
                        index = 255;
                        ui->lineEdit_input->setFocus();
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 0)
                {
                    index = 1;
                    ui->lineEdit_oldpasswd->setFocus();
                } else if (index == 1){
                    index = 2;
                    ui->lineEdit_newpasswd->setFocus();
                } else if (index == 2){
                    index = 0;
                    ui->lineEdit_old_passwd->setFocus();
                }
            } else if (verify_flag == 2){
                if(index == 255){
                    index = 254;
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
                        index = 255;
                        ui->lineEdit_input->setFocus();
                    }
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "0");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "0");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "0");
            }
            else{
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "0");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "0");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "1");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "1");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "1");
            } else {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "1");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "1");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "2");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "2");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "2");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "2");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "2");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "3");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "3");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "3");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "3");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "3");

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "4");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "4");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "4");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "4");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "4");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "5");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "5");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "5");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "5");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "5");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "6");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "6");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "6");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "6");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "6");

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "7");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "7");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "7");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "7");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "7");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "8");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "8");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "8");
            }
            else
            {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "8");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "8");
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(verify_flag == 0)
            {
                if(index == 2)
                    ui->lineEdit_newpasswd->setText(ui->lineEdit_newpasswd->text() + "9");
                else if(index == 1)
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "9");
                else if(index == 0)
                    ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "9");
            } else {
                if(index == 255)
                    ui->lineEdit_input->setText(ui->lineEdit_input->text() + "9");
                else
                    ui->lineEdit_oldpasswd->setText(ui->lineEdit_oldpasswd->text() + "9");
            }
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
            (*p_switch_flag) = false;
            this->~Dialog_UserCheckAndLogin();
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

Dialog_UserCheckAndLogin::~Dialog_UserCheckAndLogin()
{
    delete ui;
}
