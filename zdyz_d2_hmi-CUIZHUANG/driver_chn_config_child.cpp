#include "driver_chn_config_child.h"
#include "ui_driver_chn_config_child.h"

#include <QEvent>
#include <QKeyEvent>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QComboBox>
#include "./Resources/key_value.h"
#include "driver_chn_config.h"

extern QList<QPair<QString,QString>> g_key_action_match;
extern QMap<QString,int> g_driver_match;
extern QMap<QString,ACTION_MATCH_S> g_action_match;
extern QMap<QString,DRIVER_CHN_S> g_action_driver_match;

driver_chn_config_child::driver_chn_config_child(int index, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::driver_chn_config_child)
{
    ui->setupUi(this);
    this->index = index;
    ui->title_label->setText(g_key_action_match[index].first+"配置界面");
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    row = 0;
    column = 0;
    int pos = 0;
    ui->action_comboBox->addItem("无");
    QMap<QString,ACTION_MATCH_S>::iterator it = g_action_match.begin();
    if(g_key_action_match.size() == 0)
    {
        TLOG_INFO("action.xml is null");
        return;
    }
    while (it != g_action_match.end()) {
        if(it.key() == g_key_action_match[index].second)
        {
            if(it.value().occurpied == false)
                it.value().occurpied = true;
            ui->action_comboBox->addItem(it.key());
            pos = ui->action_comboBox->count()-1;
        }
        if(it.value().occurpied == false)
            ui->action_comboBox->addItem(it.key());
        it++;
    }
    select_flag = false;
    int dri_pos[8] = {0};
    ui->action_comboBox->setCurrentIndex(pos);
    old_combox_pos[0] = pos;
    ui->driver1_comboBox_1->addItem("NONE");
    ui->driver1_comboBox_2->addItem("NONE");
    ui->driver1_comboBox_3->addItem("NONE");
    ui->driver1_comboBox_4->addItem("NONE");
    ui->driver2_comboBox_1->addItem("NONE");
    ui->driver2_comboBox_2->addItem("NONE");
    ui->driver2_comboBox_3->addItem("NONE");
    ui->driver2_comboBox_4->addItem("NONE");
    QMap<QString,int>::iterator itera = g_driver_match.begin();
    if(g_driver_match.size() == 0)
    {
        TLOG_INFO("action.xml is null");
        return;
    }
    while (itera != g_driver_match.end()) {
        ui->driver1_comboBox_1->addItem(itera.key());
        ui->driver1_comboBox_2->addItem(itera.key());
        ui->driver1_comboBox_3->addItem(itera.key());
        ui->driver1_comboBox_4->addItem(itera.key());
        ui->driver2_comboBox_1->addItem(itera.key());
        ui->driver2_comboBox_2->addItem(itera.key());
        ui->driver2_comboBox_3->addItem(itera.key());
        ui->driver2_comboBox_4->addItem(itera.key());
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver1[0] == itera.key())
            dri_pos[0] = ui->driver1_comboBox_1->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver1[1] == itera.key())
            dri_pos[1] = ui->driver1_comboBox_2->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver1[2] == itera.key())
            dri_pos[2] = ui->driver1_comboBox_3->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver1[3] == itera.key())
            dri_pos[3] = ui->driver1_comboBox_4->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver2[0] == itera.key())
            dri_pos[4] = ui->driver2_comboBox_1->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver2[1] == itera.key())
            dri_pos[5] = ui->driver2_comboBox_2->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver2[2] == itera.key())
            dri_pos[6] = ui->driver2_comboBox_3->count()-1;
        if(g_action_driver_match.find(g_key_action_match[index].second).value().driver2[3] == itera.key())
            dri_pos[7] = ui->driver2_comboBox_4->count()-1;
        itera++;
    }
    ui->driver1_comboBox_1->setCurrentIndex(dri_pos[0]);
    ui->driver1_comboBox_2->setCurrentIndex(dri_pos[1]);
    ui->driver1_comboBox_3->setCurrentIndex(dri_pos[2]);
    ui->driver1_comboBox_4->setCurrentIndex(dri_pos[3]);
    ui->driver2_comboBox_1->setCurrentIndex(dri_pos[4]);
    ui->driver2_comboBox_2->setCurrentIndex(dri_pos[5]);
    ui->driver2_comboBox_3->setCurrentIndex(dri_pos[6]);
    ui->driver2_comboBox_4->setCurrentIndex(dri_pos[7]);
    for(pos = 0; pos < 8; pos++)
    {
        old_combox_pos[pos+1] = dri_pos[pos];
    }
    cur_select_combox = ui->action_comboBox;
}

driver_chn_config_child::~driver_chn_config_child()
{
    delete ui;
}

QComboBox *driver_chn_config_child::get_cur_combox(int row, int column)
{
    QComboBox *cur_combox = ui->action_comboBox;
    if(row == 0)
        return cur_combox;

    if(column == 0){
        switch (row){
        case 1:
            cur_combox = ui->driver1_comboBox_1;
            break;
        case 2:
            cur_combox = ui->driver1_comboBox_2;
            break;
        case 3:
            cur_combox = ui->driver1_comboBox_3;
            break;
        default:
            cur_combox = ui->driver1_comboBox_4;
            break;
        }
    }
    else {
        switch (row){
        case 1:
            cur_combox = ui->driver2_comboBox_1;
            break;
        case 2:
            cur_combox = ui->driver2_comboBox_2;
            break;
        case 3:
            cur_combox = ui->driver2_comboBox_3;
            break;
        default:
            cur_combox = ui->driver2_comboBox_4;
            break;
        }
    }
    return cur_combox;
}

void driver_chn_config_child::update_data(int idx, QComboBox &combobox)
{
    if(idx == 0){
        g_key_action_match[this->index].second = combobox.currentText();
        return ;
    }

    if(idx < 5){
        g_action_driver_match.find(g_key_action_match[this->index].second).value().driver1[idx-1] = combobox.currentText();
    }
    else {
        g_action_driver_match.find(g_key_action_match[this->index].second).value().driver2[idx-5] = combobox.currentText();
    }
}

void driver_chn_config_child::reload_driver_config(QString action_name)
{
    int dri_pos[8] = {0};
    TLOG_INFO("%s\n", action_name.toUtf8().data());
    QMap<QString,int>::iterator itera = g_driver_match.begin();
    if(g_driver_match.size() == 0)
    {
        TLOG_INFO("action.xml is null");
        return;
    }

    while (itera != g_driver_match.end()) {
        if(g_action_driver_match.find(action_name).value().driver1[0] == itera.key())
            dri_pos[0] = ui->driver1_comboBox_1->findText(itera.key());
            //dri_pos[0] = ui->driver1_comboBox_1->count()-1;
        if(g_action_driver_match.find(action_name).value().driver1[1] == itera.key())
            dri_pos[1] = ui->driver1_comboBox_2->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver1[2] == itera.key())
            dri_pos[2] = ui->driver1_comboBox_3->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver1[3] == itera.key())
            dri_pos[3] = ui->driver1_comboBox_4->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver2[0] == itera.key())
            dri_pos[4] = ui->driver2_comboBox_1->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver2[1] == itera.key())
            dri_pos[5] = ui->driver2_comboBox_2->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver2[2] == itera.key())
            dri_pos[6] = ui->driver2_comboBox_3->findText(itera.key());
        if(g_action_driver_match.find(action_name).value().driver2[3] == itera.key())
            dri_pos[7] = ui->driver2_comboBox_4->findText(itera.key());
        itera++;
    }
    ui->driver1_comboBox_1->setCurrentIndex(dri_pos[0]);
    ui->driver1_comboBox_2->setCurrentIndex(dri_pos[1]);
    ui->driver1_comboBox_3->setCurrentIndex(dri_pos[2]);
    ui->driver1_comboBox_4->setCurrentIndex(dri_pos[3]);
    ui->driver2_comboBox_1->setCurrentIndex(dri_pos[4]);
    ui->driver2_comboBox_2->setCurrentIndex(dri_pos[5]);
    ui->driver2_comboBox_3->setCurrentIndex(dri_pos[6]);
    ui->driver2_comboBox_4->setCurrentIndex(dri_pos[7]);
    int pos;
    for(pos = 0; pos < 8; pos++)
    {
        old_combox_pos[pos+1] = dri_pos[pos];
    }
    return ;
}

bool driver_chn_config_child::combox_changed()
{
    bool ret = false;
    ui->driver1_comboBox_1->currentIndex();
    ui->driver1_comboBox_2->currentIndex();
    ui->driver1_comboBox_3->currentIndex();
    ui->driver1_comboBox_4->currentIndex();
    ui->driver2_comboBox_1->currentIndex();
    ui->driver2_comboBox_2->currentIndex();
    ui->driver2_comboBox_3->currentIndex();
    ui->driver2_comboBox_4->currentIndex();
    int i=0;
    if(old_combox_pos[i++] != ui->action_comboBox->currentIndex()){
        ret = true;
        update_data(i-1, *ui->action_comboBox);
    }
    if(old_combox_pos[i++] != ui->driver1_comboBox_1->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver1_comboBox_1);
    }
    if(old_combox_pos[i++] != ui->driver1_comboBox_2->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver1_comboBox_2);
    }
    if(old_combox_pos[i++] != ui->driver1_comboBox_3->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver1_comboBox_3);
    }
    if(old_combox_pos[i++] != ui->driver1_comboBox_4->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver1_comboBox_4);
    }
    if(old_combox_pos[i++] != ui->driver2_comboBox_1->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver2_comboBox_1);
    }
    if(old_combox_pos[i++] != ui->driver2_comboBox_2->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver2_comboBox_2);
    }
    if(old_combox_pos[i++] != ui->driver2_comboBox_3->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver2_comboBox_3);
    }
    if(old_combox_pos[i++] != ui->driver2_comboBox_4->currentIndex()){
        ret = true;
        update_data(i-1, *ui->driver2_comboBox_4);
    }
    return ret;
}

bool driver_chn_config_child::event(QEvent *event)
{
    QString strtab;

    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type()){
    case KeyValue::KEY_RELEASED:  // key released
        comm_acs_thread->control_timer->stop();
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            if(select_flag)
                break;
            else {
                if(column == 0)
                    column = 1;
                cur_select_combox = get_cur_combox(row, column);
                cur_select_combox->setFocus();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            if(select_flag)
                break;
            else {
                if(column == 1)
                    column = 0;
                cur_select_combox = get_cur_combox(row, column);
                cur_select_combox->setFocus();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            if(select_flag)
            {
                int cur_index = cur_select_combox->currentIndex();
                if(cur_index > 0)
                    --cur_index;
                cur_select_combox->setCurrentIndex(cur_index);
                QModelIndex itemIndex = cur_select_combox->view()->model()->index(cur_index,0);
                cur_select_combox->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);
            }
            else {

                if(row > 0)
                    --row;
                cur_select_combox = get_cur_combox(row, column);
                cur_select_combox->setFocus();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            if(select_flag)
            {
                int cur_index = cur_select_combox->currentIndex();
                if(cur_index < cur_select_combox->count()-1)
                    ++cur_index;
                cur_select_combox->setCurrentIndex(cur_index);
                QModelIndex itemIndex = cur_select_combox->view()->model()->index(cur_index,0);
                cur_select_combox->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);

            }
            else {
                if(row < 4)
                    ++row;
                cur_select_combox = get_cur_combox(row, column);
                cur_select_combox->setFocus();
                cur_select_combox->raise();
            }


            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
            if(select_flag == false)
            {
                select_flag = true;
                last_pos = cur_select_combox->currentIndex();
                cur_select_combox->showPopup();
            }
            else if(select_flag == true)
            {
                select_flag = false;
                cur_select_combox->hidePopup();
                TLOG_INFO("%d  %d   %d\n", row, last_pos, cur_select_combox->currentIndex());
                if((row == 0)&&(last_pos != cur_select_combox->currentIndex()))
                    reload_driver_config(cur_select_combox->currentText());
                last_pos = cur_select_combox->currentIndex();
            }

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            if(select_flag){
                select_flag = false;
                cur_select_combox->hidePopup();
                cur_select_combox->setCurrentIndex(last_pos);
            }
            else {
                if(true == combox_changed())
                    emit update_signal();

                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                this->~driver_chn_config_child();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_MINUS:
        {
            if(select_flag)//&&(row != 0))
            {
                int cur_index = cur_select_combox->currentIndex();
                if(cur_index < cur_select_combox->count()-7)
                    cur_index += 7;
                else
                    cur_index = cur_select_combox->count()-1;
                cur_select_combox->setCurrentIndex(cur_index);
                QModelIndex itemIndex = cur_select_combox->view()->model()->index(cur_index,0);
                cur_select_combox->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);

            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_ARROW_PLUS:
        {
            if(select_flag)//&&(row != 0))
            {
                int cur_index = cur_select_combox->currentIndex();
                if(cur_index > 7)
                    cur_index -= 7;
                else
                    cur_index = 0;
                cur_select_combox->setCurrentIndex(cur_index);
                QModelIndex itemIndex = cur_select_combox->view()->model()->index(cur_index,0);
                cur_select_combox->view()->selectionModel()->setCurrentIndex(itemIndex,QItemSelectionModel::SelectCurrent);

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
