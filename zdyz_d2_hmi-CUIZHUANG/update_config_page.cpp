#include "update_config_page.h"
#include "ui_update_config_page.h"
#include "Resources/key_value.h"
#include "update_progress_page.h"
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QRect>
#include <QNetworkInterface>
//#include "host.h"
#include "./Resources/error_code.h"
#include "./window/leftpane.h"
#include "syncparafile.h"
#include "config_backup_restore.h"

extern QList <mainPage_uigonfig> menuconfig_list;
extern struct Update_info update_info;
extern int update_flag_workface;
extern bool g_master_opc_connected_flag;
extern bool g_local_opc_connected_flag;

extern bool g_support_update_flag;
extern char ip_client[16];//client ip
extern bool updating;
extern leftPane *upgrade_parameter_show;
extern QString Category_Index;
extern Gui_config guisetting;

leftPane *upgrade_parameter_show;

UpdateConfigPage::UpdateConfigPage(int second_level_page, QWidget *parent) :
     QWidget(parent), ui(new Ui::UpdateConfigPage),second_page_index(second_level_page)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    this->row = 0;
    this->index = 0;
    this->count = menuconfig_list[second_page_index].nextclass_page.size();
    for(int i = 0;i < this->count; i++)
    {
        QLabel *label = new QLabel(this);
        label->setGeometry(QRect(0, 80+55*i, 775, 55));
        label->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
        label->setAlignment(Qt::AlignRight);
        m_menu_list.push_back(label);

        QLabel *label_text = new QLabel(this);
        label_text->setGeometry(QRect(120, 80+55*i, 300, 55));
        QFont font;
        font.setFamily(QString::fromUtf8("YouSheBiaoTiHei"));
        font.setPointSize(32);
        label_text->setFont(font);
        label_text->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_text->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_text->setText(menuconfig_list[second_page_index].nextclass_page[i].page_chn);
        m_menu_list.push_back(label_text);
    }
    m_menu_list.at(this->row*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));
    m_menu_list.at(this->row*2 + 1)->move(m_menu_list.at(this->row*2 + 1)->x()-20, m_menu_list.at(this->row*2 + 1)->y());

    connect(this,SIGNAL(message_opc_unconnected(int)),comm_acs_thread,SLOT(closeAllandShowUpdate(int)));
}

UpdateConfigPage::~UpdateConfigPage()
{
    delete ui;
    //delete tableView;
    for(int i=0; i<count*2; i++)
        delete m_menu_list.at(i);
}

QString byteArrayToUnicode(const QByteArray array){
    // state用于保存转换状态，它的成员invalidChars，可用来判断是否转换成功
    // 如果转换成功，则值为0，如果值大于0，则说明转换失败
    QTextCodec::ConverterState state;
    // 先尝试使用utf-8的方式把QByteArray转换成QString
    QString text = QTextCodec::codecForName("UTF-8")->toUnicode(array.constData(), array.size(), &state);
    // 如果转换时无效字符数量大于0，说明编码格式不对
    if (state.invalidChars > 0)
    {
        // 再尝试使用GBK的方式进行转换，一般就能转换正确(当然也可能是其它格式，但比较少见了)
        text = QTextCodec::codecForName("GBK")->toUnicode(array);
    }
    return text;
}

bool UpdateConfigPage::event(QEvent *event)
{
    QString category_name = "upgradeSync";
    QString tab_name = "upgrade_para";

    QString parameter_name = "workface_setup";
    QString update_method = "update_method";
    QString parameter_range_min = "update_range_min";
    QString parameter_range_max = "update_range_max";
    QString update_medium = "update_medium";
    QString save_medium = "save_medium";

    QString label = "real_value";
    QStringList range_list;
    QString update_index;
    QString update_index_method;
    QString update_item;
    QString range_min;
    QString range_max;
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed

        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->row > 0){
                m_menu_list.at(row*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
                m_menu_list.at((row-1)*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));

                m_menu_list.at(row*2 + 1)->move(m_menu_list.at(this->row*2 + 1)->x()+20, m_menu_list.at(this->row*2 + 1)->y());
                m_menu_list.at((row-1)*2 + 1)->move(m_menu_list.at((this->row-1)*2 + 1)->x()-20, m_menu_list.at((this->row-1)*2 + 1)->y());
                --row;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(this->row < (count - 1)){
                m_menu_list.at(row*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_unselect.png")));
                m_menu_list.at((row+1)*2)->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/update_sys/menu_select.png")));

                m_menu_list.at(row*2 + 1)->move(m_menu_list.at(this->row*2 + 1)->x()+20, m_menu_list.at(this->row*2 + 1)->y());
                m_menu_list.at((row+1)*2 + 1)->move(m_menu_list.at((this->row+1)*2 + 1)->x()-20, m_menu_list.at((this->row+1)*2 + 1)->y());
                ++row;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            TLOG_INFO("%d\n", this->row);
            if(menuconfig_list[5].nextclass_page[this->row].access == 1)
            {
                if(this->row == 0){//升级参数配置
                    QString category = "upgradeSync";
                    leftPane* parameter_show = new leftPane(category);
                    parameter_show->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = parameter_show;
                } else if(this->row == 1){//升级
                    QJsonObject obj;
                    QJsonArray array;
                    QString ftp_path;
                    obj["sup_no"] = this->comm_acs_thread->cmdManager->address_set->get_local_address();

                    update_index_method = this->para->query(category_name, tab_name, update_method, label);
                    range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
                    update_index = this->para->query(category_name, tab_name, parameter_name, label);
                    update_item = range_list[update_index.toInt()];
                    range_min = this->para->query(category_name, tab_name, parameter_range_min, label);
                    range_max = this->para->query(category_name, tab_name, parameter_range_max, label);

                    QString user_data = "none";
                    if(update_index_method.toInt() == 0){
                        if(range_min.toInt() != this->comm_acs_thread->cmdManager->address_set->get_local_address())
                            user_data = range_min + QString("#");
                        array.append(range_min.toInt());
                    } else if (update_index_method.toInt() == 2){
                        if((this->comm_acs_thread->cmdManager->address_set->get_local_address() < range_min.toInt()) || \
                            (this->comm_acs_thread->cmdManager->address_set->get_local_address() > range_max.toInt())){
                            user_data = range_min + QString("#") + QString("-") + range_max + QString("#");
                        }
                        array.append(range_min.toInt());
                        array.append(range_max.toInt());
                    } else {
                        array.append(0);
                    }

                    update_process* update = new update_process(user_data);
                    update->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = update;
                    connect(comm_acs_thread->cmdManager, &CmdManager::signal_process_update, update, &update_process::process_update);
                    connect(comm_acs_thread->cmdManager, &CmdManager::signal_status_update, update, &update_process::status_update);
                    connect(comm_acs_thread->cmdManager, &CmdManager::signal_file_set, update, &update_process::file_set);
                    emit comm_acs_thread->cmdManager->signal_file_set(update_item);

                    obj["range"] = array;
                    obj["name"] = update_item;
                    ftp_path = QString("ftp://") + guisetting.update_cfg.ubang_ip + guisetting.update_cfg.file_path + update_item;
                    obj["path"] = ftp_path;
                    obj["version"] = QString("V0.0.1");

                    QJsonObject info_obj;
                    info_obj["info"] = obj;
                    //发送给本机
                    QJsonDocument json_doc(info_obj);
                    this->comm_acs_thread->send_socket_msg_to_acs(QString("update"), json_doc.toJson());
                    //发送给外部
                    QJsonObject obj_extern;
                    obj_extern["destination"] = array;
                    obj_extern["topic"] = QString("update");
                    obj_extern["message"] = info_obj;
                    QJsonDocument json_ex_doc(obj_extern);
                    this->comm_acs_thread->send_socket_msg_to_acs(QString("extern_msg"), json_ex_doc.toJson());

                    TLOG_INFO("%s\n", json_ex_doc.toJson().data());

                } else if(this->row == 2){//版本同步
                    SyncParaFile *sync_param = new SyncParaFile(TYPE_VERSION);
                    sync_param->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = sync_param;
#if 0
                    if(access("/root/sync.tar.gz", F_OK) == 0)
                        unlink("/root/sync.tar.gz");
                    system("tar -czvf /root/sync.tar.gz /userdata/bin /userdata/config");
                    update_index_method = this->para->query(category_name, tab_name, update_method, label);
                    range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
                    update_index = this->para->query(category_name, tab_name, parameter_name, label);
                    update_item = range_list[update_index.toInt()];
                    range_min = this->para->query(category_name, tab_name, parameter_range_min, label);
                    range_max = this->para->query(category_name, tab_name, parameter_range_max, label);
                    QJsonObject obj;
                    QJsonArray array;
                    QString ftp_path;
                    obj["sup_no"] = this->comm_acs_thread->cmdManager->address_set->get_local_address();
                    if(update_index_method.toInt() == 0){
                        array.append(range_min.toInt());
                    } else if (update_index_method.toInt() == 2){
                        array.append(range_min.toInt());
                        array.append(range_max.toInt());
                    } else {
                        array.append(0);
                    }
                    obj["range"] = array;
                    obj["name"] = QString("sync.tar.gz");
                    ftp_path = QString("ftp://");
                    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
                    foreach (QNetworkInterface iface, ifaces)
                    {
                        TLOG_INFO("%s:%s\n", iface.name().toUtf8().data(), iface.addressEntries().first().ip().toString().toUtf8().data());
                        if(iface.hardwareAddress() != nullptr)
                        {
                            if(iface.name().compare("eth0") == 0)
                            {
                                ftp_path += iface.addressEntries().first().ip().toString();
                                break;
                            }
                        }
                    }
                    ftp_path += QString("/root/sync.tar.gz");
                    obj["path"] = ftp_path;
                    obj["version"] = QString("V0.0.1");

                    QJsonObject info_obj;
                    info_obj["info"] = obj;
                    //QJsonDocument json_doc(info_obj);
                    //this->comm_acs_thread->send_socket_msg_to_acs(QString("update"), json_doc.toJson());
#endif
                    //发送给外部
                    QJsonObject obj_extern;
                    QJsonArray array;
                    QJsonObject obj_msg;
                    obj_msg["test"] = QString("info_obj");
                    array.append(0);
                    obj_extern["destination"] = array;
                    obj_extern["topic"] = QString("update");
                    obj_extern["message"] = obj_msg;
                    QJsonDocument json_ex_doc(obj_extern);
                    this->comm_acs_thread->send_socket_msg_to_acs(QString("extern_msg"), json_ex_doc.toJson());

                    TLOG_INFO("%s\n", json_ex_doc.toJson().data());

                }  else if(this->row == 3){//参数同步
                    TLOG_INFO("parameters sync\n");
                    SyncParaFile *sync_param = new SyncParaFile(TYPE_PARAM_DELAY);
                    sync_param->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = sync_param;
#if 0
                    if(access("/root/sync.tar.gz", F_OK) == 0)
                        unlink("/root/sync.tar.gz");
                    system("tar -czvf /root/sync.tar.gz /userdata/config");
                    update_index_method = this->para->query(category_name, tab_name, update_method, label);
                    range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
                    update_index = this->para->query(category_name, tab_name, parameter_name, label);
                    update_item = range_list[update_index.toInt()];
                    range_min = this->para->query(category_name, tab_name, parameter_range_min, label);
                    range_max = this->para->query(category_name, tab_name, parameter_range_max, label);
                    QJsonObject obj;
                    QJsonArray array;
                    QString ftp_path;
                    obj["sup_no"] = this->comm_acs_thread->cmdManager->address_set->get_local_address();
                    if(update_index_method.toInt() == 0){
                        array.append(range_min.toInt());
                    } else if (update_index_method.toInt() == 2){
                        array.append(range_min.toInt());
                        array.append(range_max.toInt());
                    } else {
                        array.append(0);
                    }
                    obj["range"] = array;
                    obj["name"] = QString("sync.tar.gz");
                    ftp_path = QString("ftp://");
                    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
                    foreach (QNetworkInterface iface, ifaces)
                    {
                        TLOG_INFO("%s:%s\n", iface.name().toUtf8().data(), iface.addressEntries().first().ip().toString().toUtf8().data());
                        if(iface.hardwareAddress() != nullptr)
                        {
                            if(iface.name().compare("eth0") == 0)
                            {
                                ftp_path += iface.addressEntries().first().ip().toString();
                                break;
                            }
                        }
                    }
                    ftp_path += QString("/root/sync.tar.gz");
                    obj["path"] = ftp_path;
                    obj["version"] = QString("V0.0.1");

                    QJsonObject info_obj;
                    info_obj["info"] = obj;
                    //QJsonDocument json_doc(info_obj);
                    //this->comm_acs_thread->send_socket_msg_to_acs(QString("update"), json_doc.toJson());

                    //发送给外部
                    QJsonObject obj_extern;
                    obj_extern["destination"] = array;
                    obj_extern["topic"] = QString("update");
                    obj_extern["message"] = info_obj;
                    QJsonDocument json_ex_doc(obj_extern);
                    this->comm_acs_thread->send_socket_msg_to_acs(QString("extern_msg"), json_ex_doc.toJson());
#endif
                } else if(this->row == 4){//配置备份
                    TLOG_INFO("config backup\n");
                    SyncParaFile *sync_param = new SyncParaFile(TYPE_CONFIG_BACKUP);
                    sync_param->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = sync_param;
                } else if(this->row == 5){//配置恢复
                    TLOG_INFO("config restore\n");
                    ConfigRestorePage *restore_page = new ConfigRestorePage();
                    restore_page->show();
                    this->widget_stack->push(this);
                    this->comm_acs_thread->cmdManager->window = restore_page;
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~UpdateConfigPage();
            TLOG_INFO("****hmi: system_config_page: key cancel");
            //this->comm_acs_thread->cmdManager->lock_ui_off();
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
