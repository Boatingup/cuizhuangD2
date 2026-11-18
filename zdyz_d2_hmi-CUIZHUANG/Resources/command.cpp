#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QString>
#include <QTimer>
#include <QRegExp>
#include <QStack>
#include <QWidget>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QNetworkInterface>
#include <qmutex.h>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "command.h"
#include "../window/mainpage.h"
#include "error_code.h"
#include "key_value.h"
#include "../Resources/guiconfig.h"
//#include "../threads/para_synchronize_file.h"
#include "../threads/comm_acs_thread.h"
#include "../threads/comm_acs_socket_thread.h"
#include "../Resources/mqtt_message.h"
#include "../show_version.h"
#include "../update_process.h"

//#define USED_XML_PATH "/userdata/config/hmi_files/driver_chn_config.xml"
#define PICTURE_SAVE_PATH "/others/screen"

extern QWidget* pMainPage;
extern QList<info_prara> readonly_tabs;
extern QString Category_Index;
extern QList<sensor_info> sensor_list;
extern QList<sensor_info> wireless_list;
extern Gui_config   guisetting;
extern QVector<QMap<QString, FAULT_INFO_S>> g_fault;
extern QMap<QString, FAULT_PARAM_S> g_fault_param;
extern QSemaphore g_sem_audio;
extern QQueue<QString> g_audio_msg;
//extern QMap<QString,QString> g_value_action_match;
extern int g_port_visualization;
extern int sub_str_to_list(QString str, char sep, char sign, QList<QPair<QString, QString>> &out_put);
extern int sub_str_to_vector(QString str, char sep, QVector<QString> &out_put);
extern void update_driver_chn_config_xml(QVector<QString> &info);
extern int g_fault_status_flag;
extern unsigned int g_stop_lock_info;
extern int g_coal_machine_fence;

QSemaphore g_sem_audio(0);
QQueue<QString> g_audio_msg;
QVector<QMap<QString, FAULT_INFO_S>> g_fault;
QMap<QString, FAULT_PARAM_S> g_fault_param;
int g_fault_status_flag = -1;
int g_human_num = -1;
unsigned int g_stop_lock_info;

CmdManager::CmdManager(){

    command_map.insert(QString("key"), new cmd_key(this));
    command_map.insert(QString("state"), new cmd_state(this));
    command_map.insert(QString("select"), new cmd_select(this));
    command_map.insert(QString("sensor"), new cmd_sensor(this));
    command_map.insert(QString("parameter_set"), new cmd_parameter(this));
    command_map.insert(QString("action"), new cmd_action(this));
    command_map.insert(QString("stop"), new cmd_stop(this));
    command_map.insert(QString("lock"), new cmd_lock(this));
    command_map.insert(QString("config"), new cmd_config(this));
    command_map.insert(QString("fault"), new cmd_system_fault(this));
    //command_map.insert(QString("update"), new cmd_system_fault(this));

    this->ss = SystemState::getInstance();
    this->address_set = AddressSet::getInstance();
    this->sensor = Sensor::getInstance();
    this->action_register = ActionRegister::getInstance();
    this->para = Parameters::getInstance();
    this->human_lock_switch = 1; //default is on.
    this->Msg_Sem = new QSemaphore(1);
    this->Pro_Sem = new QSemaphore(0);
    this->list_template.clear();
    this->m_semaphore = new QSemaphore(0);
    this->m_mutex = new QMutex();
    this->acs_version.clear();
    this->ui_lock = new QMutex(QMutex::NonRecursive);
    this->widget_stack = WidgetStack::getInstance();

    hmi_version = HMI_VERSION;

    connect(&m_timer_count,SIGNAL(timeout()), this, SLOT(cb_keep_live_count()));
    m_timer_count.stop();
}

CmdManager::~CmdManager(){
    if(!this->command_map.empty()){
        for(QMap<QString, command *>::const_iterator it = command_map.begin(); it != command_map.end(); ++it) {
            delete it.value();
        }
    }
    delete this->m_semaphore;
    delete this->m_mutex;
}

void CmdManager::RecordMemory_coal()
{
    this->para->WriteTemplate(this->list_template);

    QString category_name = "system";
    QString tab_name =  "system_para";
    QString parameter_name = "syn_template";
    QString label = "real_value";
    QString value = this->para->query(category_name, tab_name, parameter_name, label);

    //queryTemplate(QString & intervals_string,QString & id_list_string,QStringList &intervals_list,QStringList &id_list);

    QString intervals_string;
    QString id_list_string;
    QStringList intervals_list;
    QStringList id_list;
    if(value == "1")
    {
        //模板同步
        QString syn_template = "Template";
        ParaSynchronizeThread* pst = ParaSynchronizeThread::getInstance();
        if(this->para->queryTemplate(intervals_string,id_list_string,intervals_list,id_list) == 0)
        {
            QJsonObject syzk_template;
            syzk_template["steps"]     = id_list_string;
            syzk_template["intervals"] = intervals_string;

            QJsonObject one_key_coal;
            one_key_coal["syzk_template"] = syzk_template;

            QJsonObject Template_obj;
            Template_obj["one_key_coal"] = one_key_coal;

            QJsonObject param_obj;
            param_obj["Template"] = one_key_coal;

            QJsonDocument doc(param_obj);
            QString str_send = doc.toJson(QJsonDocument::Compact);

            pst->send_para_synchronization_msg(str_send);
            TLOG_INFO("send template -------%s------ ", str_send.toUtf8().constData());
        } else {
            syn_template += ":";
            syn_template += "null";
            syn_template += ":";
            syn_template += "null";
        }
    }
}
/*
void CmdManager::Log_current_queue()
{
    int i = 0;
    for(auto msg : msg_queue)
    {
        TLOG_INFO("msg %d : %s", i, msg.toStdString().c_str());
    }
}
*/

void CmdManager::lock_ui_off()
{
    if(guisetting.sys_cfg.mutex_flag  == 1)
    {
        //LOG(INFO) << "will unlock";
        this->ui_lock->unlock();
    }
    //TLOG_INFO("unlocked");
}

void CmdManager::lock_ui_on()
{
    if(guisetting.sys_cfg.mutex_flag  == 1)
    {
        //LOG(INFO) << "will lock";
        this->ui_lock->lock();
    }
    //TLOG_INFO("locked");
}

void CmdManager::split_command(QString& str_cmd){
    if(!this->cmdline.empty()){
        cmdline.clear();
    }
    //this->msgreceived = str_cmd;//保存消息用来转发

    this->cmdline = str_cmd.trimmed().split(' ', QString::SkipEmptyParts);
    return ;
}

void CmdManager::insert_msg(QString &msg)
{
    m_mutex->lock();
    m_cmd_buf.push_back(msg);
    m_mutex->unlock();
    m_semaphore->release();
}

void CmdManager::cb_keep_live_count()
{
    system("kill -9 $(pidof ffmpeg)");
    m_timer_count.stop();
}

void CmdManager::cb_master_audio_state(bool state)
{
    m_master_audio_state = state;
}

static void *wait_thread(void *param)
{
    pthread_detach(pthread_self());
    pid_t *pid_thread = static_cast<pid_t *>(param);
    waitpid(*pid_thread, nullptr, 0);
    free(param);
    return nullptr;
}

void CmdManager::json_format_switch(QString type, const QByteArray &json_data){

    /*if(!this->cmdline.empty()){
        cmdline.clear();
    }*/

    QJsonDocument document = QJsonDocument::fromJson(json_data);
    if (!document.isObject()) {
        TLOG_ERROR("JSON document is not an object ");
        return ;
    }


    QJsonObject jsonObject = document.object();
    QString buf;
    double tmp = 0.0;

    if(type == TOPIC_WILL_MSG){
        if(!jsonObject["sync"].isNull()){
            QJsonArray dev_status_array = jsonObject["sync"].toArray();
            if(!dev_status_array.isEmpty()){
                for(int i=0; i<dev_status_array.size(); i++){
                    if(m_dev_status[i] != dev_status_array[i].toBool()){
                        m_dev_status[i] = dev_status_array[i].toBool();
                    }
                }
                emit singnal_inform_status_changed(0);
            }
        }else if(!jsonObject["support_no"].isNull() && !jsonObject["online"].isNull()){
            int support_no = jsonObject["support_no"].toInt();
            if((support_no > 0) && (support_no < 256)){
                if(m_dev_status[support_no - 1] != jsonObject["online"].toBool()){
                    m_dev_status[support_no - 1] = jsonObject["online"].toBool();
                    emit singnal_inform_status_changed(support_no);
                }
            }
        }
    } else if(type == TOPIC_PORT_STATUS){
        QString name = jsonObject["name"].toString();
        QString link = jsonObject["link"].toString();
        int speed = jsonObject["speed"].toInt();
        QString master = jsonObject["master"].toString();

        int *port_info;
        if(name.contains("port10")){
            port_info = sensor->phy_info[0];
        } else if(name.contains("portleft")){
            port_info = sensor->phy_info[1];
        } else if(name.contains("portright")){
            port_info = sensor->phy_info[2];
        } else {
            return ;
        }

        port_info[0] = link.contains("up")?1:0;
        port_info[1] = speed;
        port_info[2] = master.contains("master")?1:0;

    } else if(type == TOPIC_MQTT_PROXY_STATUS){
        int iNetStatus = jsonObject["status"].toInt();

        emit CommAcsSocketThread::getInstance()->inform_net_status(iNetStatus);
    } else if(type == TOPIC_BROAD_CAST){
        for(QJsonObject::const_iterator it = jsonObject.constBegin(); it != jsonObject.constEnd(); it++)
        {
            if(it.key() == "audio")
            {
                QJsonObject audio_obj = it.value().toObject();
                QString state = audio_obj["state"].toString();
                int promoter = audio_obj["promoter"].toInt();
                int interval = audio_obj["interval"].toInt();
                if((promoter==address_set->get_local_address())||(abs(promoter-address_set->get_local_address())%interval != 0))
                    continue;
                if(state == "start"){
                    if((m_audio_flag == true)||(m_master_audio_state == true)||
                            ((ss->get_system_state() != SystemState::Normal)&&(ss->get_system_state() != SystemState::Follow)))
                        continue;
                    m_audio_flag = true;

                    QString category_name = "healthy";
                    QString tab_name =  "system_set";
                    QString parameter_name = "network_segment";
                    QString label = "real_value";
                    QString network_segment = this->para->query(category_name, tab_name, parameter_name, label);
                    if(network_segment.isNull() || network_segment.toInt() < 0 || network_segment.toInt() > 255)
                        network_segment = QString("1");

                    QString local_ip;
                    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
                    foreach (QNetworkInterface iface, ifaces)
                    {
                        if(iface.hardwareAddress() != nullptr)
                        {
                            if(iface.name().compare("eth0") == 0)
                            {
                                local_ip = iface.addressEntries().first().ip().toString();
                                break;
                            }
                        }
                    }
                    QStringList local_ip_list = local_ip.split('.');

                    QString rtsp_addr = QString("rtsp://") + local_ip_list.at(0) + QString(".") + local_ip_list.at(1) + QString(".") + \
                        network_segment + QString(".") + QString::number(promoter) + QString(":8554/live");
                    pid_t pid = fork();
                    if (pid < 0) {
                        TLOG_ERROR("StartBroadServerCmd fork process error");
                        fflush(stdout);
                    }else if (pid == 0) {
                        execlp("/usr/bin/ffmpeg","ffmpeg", "-i", rtsp_addr.toUtf8().data(), "-f", "alsa", "default", NULL);

                    }else {
                        pthread_t wait_pid;
                        pid_t *thread_pid = static_cast<pid_t *>(malloc(sizeof(pid_t)));
                        *thread_pid = pid;
                        pthread_create(&wait_pid, nullptr, wait_thread, thread_pid);

                        //m_timer_count.start(m_audio_count);
                    }
                }else if(state == "stop"){
                    if(m_audio_flag == false)
                        continue;
                    m_audio_flag = false;
                    //system("kill -9 $(pidof ffmpeg)");
                    m_timer_count.stop();
                }else if(state == "keeplive"){
                    m_timer_count.start(m_audio_count);
                }
            }
        }
    }else if(type == TOPIC_FALUT){
        for(QJsonObject::const_iterator it = jsonObject.constBegin(); it != jsonObject.constEnd(); it++)
        {
            if(it.key() == "warnlist"){
                int ori_size = g_fault.size();
                g_fault.clear();
                g_fault.resize(ori_size);
                QJsonArray array = it.value().toArray();
                for(int i=0; i<array.size(); i++){

                    QJsonObject obj_signal = array.at(i).toObject();
                    QString desp = obj_signal["Name"].toString();

                    FAULT_INFO_S fault_info;
                    fault_info.fault_diagnosis.clear();
                    fault_info.fault_display = g_fault_param.value(desp).descriptor;
                    fault_info.index = obj_signal["ID"].toInt();

                    fault_info.fault_diagnosis += g_fault_param.value(desp).dignosis;
                    fault_info.fault_time = time(nullptr);
                    fault_info.fault_occured_time = obj_signal["Time"].toString();

                    if(g_fault_param.find(desp) == g_fault_param.constEnd())
                        continue;

                    if(g_fault.at(g_fault_param.value(desp).pri).contains(desp) == true){
                        g_fault[g_fault_param.value(desp).pri].remove(desp);
                    }
                    g_fault[g_fault_param.value(desp).pri].insert(desp, fault_info);
                }
            }
            /*
            else if(it.key() == "delete"){

                for(QJsonObject::const_iterator it_signal = it.value().toObject().constBegin(); it != it.value().toObject().constEnd(); it_signal++)
                {
                    QJsonObject obj_signal = it_signal.value().toObject();
                    QString desp = obj_signal["Description"].toString();

                    if(g_fault_param.contains(desp) == false)
                            continue;
                    if(g_fault.at(g_fault_param.value(desp).pri).contains(desp) == false)
                        continue;
                    g_fault[g_fault_param.value(desp).pri].remove(desp);

                }
            }*/else{

            }
        }
        int fault_level = 0;
        for (int i=0; i<g_fault.size(); i++) {
            if(g_fault.at(i).size() == 0)
                continue;
            fault_level = i+1;
        }
        emit CommAcsSocketThread::getInstance()->inform_fault_status(fault_level);

    } else if(type == TOPIC_PARAMETER_SET){
        QJsonObject param = jsonObject["get"].toObject();
        if(param.isEmpty())
            return ;
        QString category_name;
        QString tab_name;
        QString parameter_name;
        QString label_real = "real_value";
        QString new_value = "none";
        QString temp_value;
        int value;
        bool should_sync_xml = false;
//            for(QJsonObject::const_iterator it_basic = it.value().toObject().constBegin(); it_basic != it.value().toObject().constEnd(); it_basic++)
        for(QJsonObject::const_iterator it_basic = param.constBegin(); it_basic != param.constEnd(); it_basic++)
        {
            category_name = it_basic.key();
            for(QJsonObject::const_iterator it_categpry = it_basic.value().toObject().constBegin(); it_categpry != it_basic.value().toObject().constEnd(); it_categpry++)
            {
                tab_name = it_categpry.key();   //port_config
                for(QJsonObject::const_iterator it_tab = it_categpry.value().toObject().constBegin(); it_tab != it_categpry.value().toObject().constEnd(); it_tab++)
                {
                    parameter_name = it_tab.key(); //

                    if(it_tab.value().isObject())
                    {
                        QJsonObject portObj = it_tab.value().toObject();
                        QString temp_value;
                        int value;
                        QString new_value;

                        if (portObj.contains("mode"))
                        {
                            temp_value = portObj["mode"].toString();
                            value = para->query_element_index(category_name, tab_name, parameter_name, temp_value);
                            if(value == -1)
                                continue;
                            new_value = QString::number(value);
                            para->update_cache(category_name, tab_name, parameter_name, label_real, new_value);
                        }

                        if(portObj.contains("baudrate"))
                        {
                            QString baudrate = "baudrate_value";
                            int baudrateCacheValue = portObj["baudrate"].toInt();
                            QString baudrate_value = "";
                            if (baudrateCacheValue == 115200) {
                                baudrate_value = "1";
                            } else {
                                baudrate_value = "0";
                            }
                            para->update_cache(category_name, tab_name, parameter_name, baudrate, baudrate_value);
                        }
                    } else {
                        new_value = QString::number(it_tab.value().toInt());
                        //DBG_ERR("%s %s %s %d\n", category_name.toUtf8().data(), tab_name.toUtf8().data(), parameter_name.toUtf8().data(), new_value.toInt());
                        para->update_cache(category_name, tab_name, parameter_name, label_real, new_value);
                        if(!(category_name == "healthy" && tab_name == "system_set"
                             && (parameter_name == "ntp_server_addr" || parameter_name == "time_sync_en"))) {
                            should_sync_xml = true;
                        }

                        if(category_name == "system" && tab_name == "system_para")
                        {
                            if(parameter_name == "hydraulic_supporter_type"){
                                emit CommAcsSocketThread::getInstance()->cmdManager->signal_update_sensor_data(mainPage::View_SUP_TYPE);
                            } else if (parameter_name == "minAddress") {
                                CommAcsSocketThread::getInstance()->cmdManager->m_min_address = new_value.toInt();
                            } else if (parameter_name == "maxAddress") {
                                CommAcsSocketThread::getInstance()->cmdManager->m_max_address = new_value.toInt();
                            }
                        } else if(category_name == "healthy" && tab_name == "system_set" && parameter_name == "server_addr"){
                            CommAcsSocketThread::getInstance()->cmdManager->m_route_address = new_value.toInt();
                            QString ip_value = it_tab.value().toString();
                            if (ip_value == "")  para->update_cache(category_name, tab_name, parameter_name, label_real, new_value);
                            else para->update_cache(category_name, tab_name, parameter_name, label_real, ip_value);
                        }
                    }
                }
            }
        }

        if(new_value != "none" || should_sync_xml){
            para->update_sync("/userdata/config/hmi_files/parameters.xml");
            ActionRegister::getInstance()->reload();
        }

    } else if (type == TOPIC_REBOOT){
        int time = 1;
        time = jsonObject["time"].toInt();
        sleep(time);
        system("reboot");
    } else if (type == TOPIC_SCREEN){
        QPixmap screenshot = this->window->grab();
        QString picture_name;

        QString name = jsonObject["name"].toString();

        if(access(PICTURE_SAVE_PATH, F_OK) != 0)
            mkdir(PICTURE_SAVE_PATH, 0777);

        if(!name.isNull())
            picture_name = PICTURE_SAVE_PATH + QString("/screen_") + name + QString(".png");
        else
            picture_name = PICTURE_SAVE_PATH + QString("/screen.png");

        if(access(picture_name.toUtf8().data(), F_OK) == 0)
            unlink(picture_name.toUtf8().data());
        screenshot.save(picture_name);

    } else if (type == TOPIC_KEY) {
        for(QJsonObject::const_iterator it = jsonObject.constBegin(); it != jsonObject.constEnd(); it++)
        {
            buf.clear();
            buf += QString(TOPIC_KEY) + QString(" -");
            buf += it.key();
            buf += " ";
            buf += QString::number(it.value().toInt());
            buf += " null";
            insert_msg(buf);

        }
    } else if (type == TOPIC_PARAMETER_EXTERN_SET) {
        ParaSynchronizeThread::getInstance()->parse_msg1(json_data);
    } else if (type == TOPIC_UPDATE){
        QJsonObject ack_info = jsonObject["ack"].toObject();
        QJsonObject result_info = jsonObject["result"].toObject();
        QJsonObject update_info = jsonObject["info"].toObject();

        QString test_info = jsonObject["test"].toString();
        TLOG_INFO("FIX_SYNC_WINDOW 445 \n");
        if(!test_info.isNull()){
            TLOG_INFO("FIX_SYNC_WINDOW 447\n");
            update_process* update = new update_process();
            update->show();
            this->widget_stack->push(this->window);
            this->window = update;
        }

        if(!update_info.isEmpty()){
            QString category_name = "upgradeSync";
            QString tab_name = "upgrade_para";
            QString parameter_name = "workface_setup";
            QString update_method = "update_method";
            QString label = "real_value";
            QStringList range_list;
            QString update_index;
            QString update_index_method;
            QString update_item;

            const int local_id = address_set->get_local_address();
            if(update_info["sup_no"].toInt() == local_id)
                return ;
            /* 与 zdyz_d2_update 一致：不在 range 内的架不弹升级界面 */
            QJsonArray dns = update_info["range"].toArray();
            if((dns.size() == 1) && (dns.at(0).toInt() != 0) && (dns.at(0).toInt() != local_id))
                return ;
            if((dns.size() == 2) && ((local_id < dns.at(0).toInt()) || (local_id > dns.at(1).toInt())))
                return ;

            update_process* update = new update_process();
            update->show();
            this->widget_stack->push(this->window);
            this->window = update;

            connect(this, &CmdManager::signal_process_update, update, &update_process::process_update);
            connect(this, &CmdManager::signal_status_update, update, &update_process::status_update);
            connect(this, &CmdManager::signal_file_set, update, &update_process::file_set);

            update_index_method = this->para->query(category_name, tab_name, update_method, label);
            range_list = this->para->query_range_set(category_name, tab_name, parameter_name);
            update_index = this->para->query(category_name, tab_name, parameter_name, label);
            //update_item = range_list[update_index.toInt()];
            update_item = update_info["name"].toString();
            emit signal_file_set(update_item);

        }else if(!ack_info.isEmpty()){
            int sup_no = ack_info["sup_no"].toInt();
            if(sup_no != address_set->get_local_address())
                return ;

            if(ack_info["ack"] == 2){
                emit signal_status_update("拒绝升级");
            } else {
                emit signal_status_update("升级中");
            }

        } else if (!result_info.isEmpty()){
            int sup_no = result_info["sup_no"].toInt();
            if(sup_no != address_set->get_local_address())
                return ;

            if(result_info["result"] == 0){
                emit signal_status_update("升级中");
                emit signal_process_update(result_info["received"].toDouble(), result_info["total"].toDouble());
            } else if(result_info["result"] == 1) {
                emit signal_status_update("下载成功");
            } else if(result_info["result"] == 2) {
                emit signal_status_update("升级成功");
            } else {
                emit signal_status_update(QString("升级失败,")+result_info["info"].toString());
            }
        }

    } else if (type == TOPIC_CONFIG){
        for(QJsonObject::const_iterator itera = jsonObject.constBegin(); itera != jsonObject.constEnd(); itera++){
            if(itera.key() == "support_number") {
                buf += QString(TOPIC_CONFIG) + QString(" -l ") + QString::number(itera.value().toInt());
                insert_msg(buf);
            } else if (itera.key() == "acs_version") {
                m_acs_version = itera.value().toString();
            } else if (itera.key() == "opc_version") {
                m_opc_version = itera.value().toString();

            } else if (itera.key() == "mqtt_proxy_version") {
                m_mqtt_version = itera.value().toString();

            } else if (itera.key() == "datacap_version") {
                m_data_version = itera.value().toString();

            } else if (itera.key() == "healthmanager_version") {
                m_health_version = itera.value().toString();

            } else if (itera.key() == "keyboard_version") {
                m_key_version = itera.value().toString();

            } else if (itera.key() == "update_version") {
                m_update_version = itera.value().toString();

            } else if (itera.key() == "sbc1_version") {
                m_sbc_1_version = itera.value().toString();

            } else if (itera.key() == "sbc2_version") {
                m_sbc_2_version = itera.value().toString();

            } else if (itera.key() == "coalboard_version") {
                m_coal_board_version = itera.value().toString();

            } else if (itera.key() == "signalconvert_version") {
                m_signal_version = itera.value().toString();

            } else if (itera.key() == "get") {
                if(!itera.value().isNull() && itera.value() != "hmi_version")
                    continue;

                QJsonObject json;
                json["hmi_version"] = hmi_version;
                QJsonDocument jsonDoc(json);
                emit singnal_send_hmi_version(jsonDoc.toJson());
            } else if (itera.key() == "infrared_node_status") {
                QString SensorName = "InfraredNodeStatus";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
            }
        }
    } else if (type == TOPIC_SELECT){
        unsigned int view = 0;
        QJsonValue range_info = jsonObject["range"];
        QJsonValue direction_info = jsonObject["direction"];
        DBG_ERR("%d\n", range_info.toArray().size());
        if(!range_info.isNull())
        {
            if(range_info.toArray().size() == 1){
                this->address_set->set_destination_address(static_cast<uint8_t>(range_info.toArray().at(0).toInt()));
            } else {
                uint8_t start = range_info.toArray().at(0).toInt();
                uint8_t end = range_info.toArray().at(1).toInt();
                this->address_set->set_destination_address(start, end);
            }
        }
        if(!direction_info.isNull())
        {
            if(direction_info.toString() == "left_to_right"){
                this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_LEFT_TO_RIGHT);
            }else{  //right_to_left
                this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_RIGHT_TO_LEFT);
            }
        } else {
            this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_GROUP);
        }
        view |= mainPage::View_Controlled_Address;
        emit this->signal_update_action_data(view);
    } else if (type == TOPIC_ACTION){
        for(QJsonObject::const_iterator it = jsonObject.constBegin(); it != jsonObject.constEnd(); it++)
        {
            buf.clear();
            if(it.key() == "action_info"){
                QJsonArray array = it.value().toArray();
                buf = QString(TOPIC_ACTION) + " ";
                if(array.size() == 0){
                    buf += QString("stop");
                    insert_msg(buf);
                    continue;
                }
                buf += QString("action_info") + " ";
                int i;
                for(i=0; i<array.size(); i++)
                {
                    buf += array.at(i).toObject().keys().at(0) + " ";
                    buf += array.at(i).toObject().value(array.at(i).toObject().keys().at(0)).toString() + " ";
                }
                DBG_ERR("%s\n", buf.toUtf8().data());
                insert_msg(buf);
            }
        }
    } else if (type == TOPIC_STATE){
        QJsonValue set_info = jsonObject["set"];
        QJsonValue stop_info = jsonObject["StopLockID"];
        QJsonValue select_info = jsonObject["select"];
        QJsonValue human_num = jsonObject["human_lock_info"];
        QJsonValue master = jsonObject["MasterControlID"];
        QJsonValue follow_status = jsonObject["follow_status"];
        QJsonValue follow_stage = jsonObject["follow_stage"];
        if(!set_info.isNull()){
            if((set_info.toString() == "Stop")|| (set_info.toString() == "Lock") || (set_info.toString() == "HumanLock"))
            {
                if(!stop_info.isNull())
                {
                    g_stop_lock_info = stop_info.toInt();

                }

                if(!human_num.isNull())
                {
                    g_human_num = human_num.toArray().size();
                }
            }
            buf += QString(TOPIC_STATE) + QString(" -set ");
            buf += set_info.toString();
            insert_msg(buf);
        }

        if(!master.isNull()){
            sensor->master_no = master.toInt();
        }

        if(!follow_status.isNull()){
            buf.clear();
            buf = QString("action -fs ") + follow_status.toString();
            insert_msg(buf);
        }

        if(!follow_stage.isNull()){
            buf.clear();
            buf = QString("action -ft ") + follow_stage.toString();
            insert_msg(buf);
        }

        if(!select_info.isNull())
        {
            unsigned int view = 0;
            QJsonValue range_info = select_info.toObject()["range"];
            QJsonValue direction_info = select_info.toObject()["direction"];
            if(!range_info.isNull())
            {
                if(range_info.toArray().size() == 1){
                    this->address_set->set_destination_address(static_cast<uint8_t>(range_info.toArray().at(0).toInt()));
                } else {
                    uint8_t start = range_info.toArray().at(0).toInt();
                    uint8_t end = range_info.toArray().at(1).toInt();
                    this->address_set->set_destination_address(start, end);
                }
            }
            if(!direction_info.isNull())
            {
                if(direction_info.toString() == "left_to_right"){
                    this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_LEFT_TO_RIGHT);
                }else{  //right_to_left
                    this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_RIGHT_TO_LEFT);
                }
            } else {
                this->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_GROUP);
            }
            view |= mainPage::View_Controlled_Address;
            emit this->signal_update_action_data(view);
        }

    } else if (type == TOPIC_SENSOR){
        QJsonObject object;
        QJsonArray array;
        unsigned int view = 0;
        int i;
        for(QJsonObject::const_iterator itera = jsonObject.constBegin(); itera != jsonObject.constEnd(); itera++){
            if(itera.key() == "extern"){

                if((this->ss->get_system_state() != SystemState::Neighbour) && (this->ss->get_system_state() != SystemState::Interval))
                    return;
                QJsonObject json = itera.value().toObject();
                QJsonValue src = json["support_no"];
                if(src.toInt() != this->address_set->get_destination_address())
                    return ;

                array = json["pressure"].toArray();
                if(!array.isEmpty()){
                    for(i=0; i<array.size(); i++)
                    {
                        object = array.at(i).toObject();
                        for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                        {
                            int index = -1;
                            index = it.key().toInt()+1;
                            sensor->extern_pressure[index] = it.value().toDouble();
                        }
                    }
                }

                array = json["trip"].toArray();
                if(!array.isEmpty()){
                    for(i=0; i<array.size(); i++)
                    {
                        object = array.at(i).toObject();
                        for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                        {
                            int index = -1;
                            index = it.key().toInt()+1;
                            sensor->extern_trip[index] = it.value().toDouble();
                        }
                    }
                }
                view |= mainPage::View_EXTERN_SENSOR;
            }else if(itera.key() == "pressure"){
                array = itera.value().toArray();
                for(i=0; i<array.size(); i++)
                {
                    object = array.at(i).toObject();
                    for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                    {
                        int index = -1;
                        index = it.key().toInt()+1;
                        tmp = it.value().toDouble();
                        if(sensor->pressure[index] != tmp)
                        {
                            sensor->pressure[index] = tmp;
                            QString SensorName = "PressureSensor" + QString::number(index);
                            QString SensorValue = QString::number(sensor->pressure[index]);
                            int rtn = this->findSensorIndexByName(SensorName,index);
                            if(rtn == 1)
                            {
                                sensor_list[index].name = SensorName;
                                sensor_list[index].value = SensorValue;
                            }
                            view |= mainPage::View_Pressure;
                        }
                        //TLOG_INFO("pressure (%d %f)", index, tmp);
                    }
                }
            } else if (itera.key() == "trip"){
                array = itera.value().toArray();
                for(i=0; i<array.size(); i++)
                {
                    object = array.at(i).toObject();
                    for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                    {
                        int index = -1;
                        index = it.key().toInt()+1;
                        tmp = it.value().toDouble();

                        if(sensor->stroke[index] != tmp)
                        {
                            sensor->stroke[index] = it.value().toDouble();
                            QString SensorName = "Travelsensor" + QString::number(index);
                            QString SensorValue = QString::number(sensor->stroke[index]);
                            int rtn = this->findSensorIndexByName(SensorName,index);
                            if(rtn == 1)
                            {
                                sensor_list[index].name = SensorName;
                                sensor_list[index].value = SensorValue;
                            }
                            view |= mainPage::View_Stroke;
                        }                        
                    }
                }
            } else if (itera.key() == "dip_angle"){
                array = itera.value().toArray();

                for(i=0; i<array.size(); i++)
                {
                    object = array.at(i).toObject();
                    for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                    {
                        QJsonArray angle_array = it.value().toArray();
                        QString SensorValue;
                        int index = -1;
                        index = it.key().toInt();
                        this->sensor->dip_angle[index][0] = angle_array.at(0).toDouble();
                        this->sensor->dip_angle[index][1] = angle_array.at(1).toDouble();
                        this->sensor->dip_angle[index][2] = angle_array.at(2).toDouble();
                        QString SensorName = "Inclinationsensor" + QString::number(index + 1);
                        int rtn = this->findSensorIndexByName(SensorName,index);
                        if(rtn == 1)
                        {
                            sensor_list[index].name = SensorName;
                            sensor_list[index].dip_value[0] = QString::number(this->sensor->dip_angle[it.key().toInt()][0], 'f', 1);
                            sensor_list[index].dip_value[1] = QString::number(this->sensor->dip_angle[it.key().toInt()][1], 'f', 1);
                            sensor_list[index].dip_value[2] = QString::number(this->sensor->dip_angle[it.key().toInt()][2], 'f', 1);
                            SensorValue = sensor_list[index].dip_value[0] + " " + sensor_list[index].dip_value[1] + " " + sensor_list[index].dip_value[2];
                            sensor_list[index].value = SensorValue;
                        }
                        view |= mainPage::View_Dip_Angle;
                    }
                }
            } else if (itera.key() == "coal_machine_info"){
                QJsonObject json = itera.value().toObject();
                QJsonValue dir_value = json["direction"];
                QJsonValue speed_value = json["speed"];
                QJsonValue pos_value = json["position"];
                if(dir_value.isString()){
                    if(dir_value.toString() == "up"){
                        this->sensor->coal_mechine_direction = 1;
                    } else if(dir_value.toString() == "down"){
                        this->sensor->coal_mechine_direction = 2;
                    } else{
                        this->sensor->coal_mechine_direction = 0;
                    }
                }

                if(speed_value.isDouble()){
                    this->sensor->coal_mechine_speed = speed_value.toDouble();
                }

                if(pos_value.isDouble()){
                    this->sensor->coal_mechine_position = speed_value.toDouble();
                }
                view |= mainPage::View_COAL_INFO;
            } else if (itera.key() == "infrared"){
                this->sensor->infra_red = itera.value().toInt();
                QString SensorName = "Infraredsensor" + QString::number(1);
                QString SensorValue = itera.value().toString();
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
            } else if (itera.key() == "coal_machine_position"){
                this->sensor->shearer_position = itera.value().toInt();
                view |= mainPage::View_Shearer_Position;
                emit this->signal_update_sensor_data(view);

                if(g_coal_machine_fence == -1){
                    continue;
                }

                QString category_name = "healthy";
                QString tab_name =  "system_set";
                QString parameter_name = "network_segment";
                QString label = "real_value";
                QString network_segment = this->para->query(category_name, tab_name, parameter_name, label);
                if(network_segment.isNull() || network_segment.toInt() < 0 || network_segment.toInt() > 255)
                    network_segment = QString("1");

                QString local_ip;
                QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
                foreach (QNetworkInterface iface, ifaces)
                {
                    if(iface.hardwareAddress() != nullptr)
                    {
                        if(iface.name().compare("eth0") == 0)
                        {
                            local_ip = iface.addressEntries().first().ip().toString();
                            break;
                        }
                    }
                }
                QStringList local_ip_list = local_ip.split('.');

                QString rtsp_addr = QString("rtsp://") + local_ip_list.at(0) + QString(".") + local_ip_list.at(1) + QString(".") + \
                        network_segment + QString(".") + QString::number(this->address_set->get_local_address()) + QString(":8554/live");

                if((this->sensor->shearer_position < this->address_set->get_local_address() + g_coal_machine_fence + 1) \
                    && (this->sensor->shearer_position > this->address_set->get_local_address() - g_coal_machine_fence - 1)){

                    if(m_rtsp_server_start_flag == true){
                        continue;
                    }
                    m_rtsp_server_start_flag = true;
#if 1
                    pid_t pid = fork();
                    if (pid < 0) {
                        TLOG_ERROR("StartBroadServerCmd fork process error");
                        fflush(stdout);
                    }else if (pid == 0) {
                        execlp("/userdata/bin/alsa_rtsp_server","alsa_rtsp_server", "hw:0,0", nullptr);//, this->address_set->get_local_address(), NULL);
                        exit(-1);
                    }else {
                        pthread_t wait_pid;
                        pid_t *thread_pid = static_cast<pid_t *>(malloc(sizeof(pid_t)));
                        *thread_pid = pid;
                        pthread_create(&wait_pid, nullptr, wait_thread, thread_pid);
                    }
                    QJsonObject audio_obj, child_obj;

                    child_obj["flag"] = 1;
                    child_obj["addr"] = rtsp_addr;
                    audio_obj["audio_shearer"] = child_obj;

                    QJsonDocument jsonDoc(audio_obj);
                    CommAcsSocketThread::getInstance()->m_client->publish(QString(TOPIC_BROAD_CAST), jsonDoc.toJson());
#endif
                } else {
                    if(m_rtsp_server_start_flag == true){
                        QJsonObject audio_obj, child_obj;
                        child_obj["flag"] = 0;
                        child_obj["addr"] = rtsp_addr;
                        audio_obj["audio_shearer"] = child_obj;
                        QJsonDocument jsonDoc(audio_obj);
                        CommAcsSocketThread::getInstance()->m_client->publish(QString(TOPIC_BROAD_CAST), jsonDoc.toJson());

                        system("kill -9 $(pidof alsa_rtsp_server)");
                        m_rtsp_server_start_flag = false;
                    }
                }

            } else if (itera.key() == "height"){
                this->sensor->height = itera.value().toInt();
                if(sensor->height)
                    this->sensor->calc_hight_flag = true;
                else
                    this->sensor->calc_hight_flag = false;
                QString SensorName = "HighSensor";
                QString SensorValue = itera.value().toString();
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_Height;
            } else if (itera.key() == "total_volt"){
                QString SensorName = "ConnectTotalVolt";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "total_current"){
                QString SensorName = "ConnectTotalCur";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "driver1_volt"){
                QString SensorName = "DriverVolt";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "driver1_current"){
                QString SensorName = "DriverCur";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "driver2_volt"){
                QString SensorName = "DriverVolt2";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "driver2_current"){
                QString SensorName = "DriverCur2";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_All;
            } else if (itera.key() == "ad_data"){
                array = itera.value().toArray();
                for(i=0; i<array.size(); i++)
                {
                    this->AD[i] = array.at(i).toInt();
                }
            } else if (itera.key() == "temperature"){
                tmp = itera.value().toDouble();

                if(tmp != this->sensor->temperature)
                {
                    this->sensor->temperature = itera.value().toDouble();

                    QString SensorName = "TemperatureSensor" + QString::number(1);
                    QString SensorValue = QString::number(itera.value().toDouble());
                    int index = -1;
                    int rtn = this->findSensorIndexByName(SensorName,index);
                    if(rtn == 1)
                    {
                        sensor_list[index].name = SensorName;
                        sensor_list[index].value = SensorValue;
                    }
                    view |= mainPage::View_Temperature;
                }
                
            } else if (itera.key() == "humidity"){
                tmp = itera.value().toDouble();

                if(tmp != this->sensor->humidity)
                {
                    this->sensor->humidity = tmp;
                    QString SensorName = "Humiditysensor" + QString::number(1);
                    QString SensorValue = QString::number(itera.value().toDouble());
                    int index = -1;
                    int rtn = this->findSensorIndexByName(SensorName,index);
                    if(rtn == 1)
                    {
                        sensor_list[index].name = SensorName;
                        sensor_list[index].value = SensorValue;
                    }
                    view |= mainPage::View_Humidity;
                }
                
            } else if (itera.key() == "pitch_angle"){
                this->sensor->pitch_angle =itera.value().toDouble();
                QString SensorName = "Pitchanglesensor";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_PITCH_ANGLE;
            } else if (itera.key() == "roll_angle"){
                this->sensor->roll_angle =itera.value().toDouble();
                QString SensorName = "Rollanglesensor";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_ROLL_ANGLE;
            }  else if (itera.key() == "wireless_stat"){

                QString SensorName, SensorValue;
                QJsonObject obj_child, obj_info;
                object = itera.value().toObject();
                for(QJsonObject::const_iterator it = object.constBegin(); it != object.constEnd(); it++)
                {
                    QJsonArray array = it.value().toArray();
                    for(i=0; i<array.size(); i++)
                    {
                        obj_child = array.at(i).toObject();
                        for(QJsonObject::const_iterator it_1 = obj_child.constBegin(); it_1 != obj_child.constEnd(); it_1++)
                        {
                            obj_info = it_1.value().toObject();
                            for(QJsonObject::const_iterator it_2 = obj_info.constBegin(); it_2 != obj_info.constEnd(); it_2++)
                            {
                                SensorName = it.key() + QString::number(it_1.key().toInt() + 1) + QString("_") + it_2.key();
                                SensorValue = QString::number(it_2.value().toInt());
                                int index = -1;
                                int rtn = this->findWirelessByName(SensorName,index);
                                if(rtn == 1)
                                {
                                    wireless_list[index].name = SensorName;
                                    wireless_list[index].value = SensorValue;
                                }
                            }
                        }
                    }
                }
            } else if (itera.key() == "near"){

            } else if (itera.key() == "human_lock_info"){

            } else if (itera.key() == "dust_sensor"){
                this->sensor->dust_concentration =itera.value().toDouble();
                QString SensorName = "DustSensor";
                QString SensorValue = QString::number(itera.value().toDouble());
                int index = -1;
                int rtn = this->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                view |= mainPage::View_DUST;
            } else {

            }
            emit this->signal_update_sensor_data(view);
        }
    }
    return ;
}

int CmdManager::findSensorIndexByName(QString name,int &index)
{

    for(int i = 0;i < sensor_list.size();i++)
    {
        if(sensor_list[i].name == name)
        {
            index = sensor_list[i].index;
            return 1;
        }
    }
    return 0;
}

int CmdManager::findWirelessByName(QString name,int &index)
{
    for(int i = 0;i < wireless_list.size();i++)
    {
        if(wireless_list[i].name == name)
        {
            index = wireless_list[i].index;
            return 1;
        }
    }
    return 0;
}

int CmdManager::findIndexByName(QString name,int &index)
{
    for(int i = 0;i < readonly_tabs.size();i++)
    {
        if(readonly_tabs[i].name == name)
        {
            index = readonly_tabs[i].tab;
            return 1;
        }
    }
    return 0;
}

void CmdManager::parse_command() {
    QStringList cmdline = this->cmdline;
    QString cmd = cmdline.at(0);
    command_map.find(cmd).value()->func(cmdline);
}

void CmdManager::parse_command(QStringList cmdline) {
    QString cmd = cmdline.at(0);
    command_map.find(cmd).value()->func(cmdline);
}

bool CmdManager::IsInUrGent()
{
    if((this->ss->get_system_state() == SystemState::Lock) || (this->ss->get_system_state() == SystemState::Stop) || (this->ss->get_system_state() == SystemState::Human_lock))
        return 1;
    else {
        return 0;
    }
}
int CmdManager::numberof_connects_systemdefault()
{
    return receivers(SIGNAL(signal_update_system_fault(QString,QString)));
}


command::command(QString cmd_name, CmdManager* pcm): name(cmd_name), cmdManager(pcm){
}
command::~command(){}


void cmd_key::func(const QStringList& cmdline){
    int key_value;
    if(cmdline.size() < 3)
    {
        TLOG_INFO("cmd invalid size");
    }

    key_value  = cmdline[2].toInt();
    if((key_value > 31) || (key_value < 0))
    {
        TLOG_INFO("key invalid ");
        this->cmdManager->lock_ui_off();
        return;
    }
    if(cmdline[1] == "-down")
    {
        if(this->cmdManager->is_show_auxiliary == false)
        {
            QCoreApplication::postEvent(this->cmdManager->window, new QKeyEvent(QEvent::User, key_value, Qt::NoModifier));
            this->cmdManager->lock_ui_off();
        }
        else {
            TLOG_INFO("start receive key down from  %s", this->cmdManager->window->objectName().toStdString().c_str());
            QCoreApplication::postEvent(this->cmdManager->window, new QKeyEvent(QEvent::User, key_value, Qt::NoModifier));
        }
    }
    else if(cmdline[1] == "-up")
    {
        TLOG_INFO("start receive key up from  %s", this->cmdManager->window->objectName().toStdString().c_str());
        QCoreApplication::postEvent(this->cmdManager->window, new QKeyEvent(static_cast<QEvent::Type>(QEvent::User+1), key_value, Qt::NoModifier));
    }
    else {
        this->cmdManager->lock_ui_off();
        TLOG_INFO("info not known");
    }
}

void cmd_state::func(const QStringList& cmdline){
    int index = 1;
    QString state = "";
    while(index < cmdline.size()){
        if(cmdline.at(index) == "-set"){
            index++;
            state = cmdline[index++];
            emit this->cmdManager->signal_update_system_state(state);
        }else{
            index++;
        }
    }
}

void cmd_sensor::func(const QStringList& cmdline){
    int index = 1;
    int index_temp;
    int data_length = 0;
    unsigned int view = 0;
    //int status = 0;

    QRegExp exp("^(-?[0-9])|(-?\\d+)(.?\\d+)$");

    while(index < cmdline.size()){
        if(cmdline.at(index) == "-p"){ //pressure sensor data, unit MPa
            index_temp = index + 1;
            while(index_temp < cmdline.size() && !(cmdline[index_temp].startsWith('-'))){
                index_temp++;
            }
            TLOG_INFO("data_length:  %d", data_length);
            data_length = index_temp - index - 1;
            for(int i=0; i<data_length; i++){  //pressure stored begins at index of 1
                this->cmdManager->sensor->pressure[i+1] = cmdline[index + i + 1].toFloat();
                QString SensorName = "PressureSensor" + QString::number(i+1);
                QString SensorValue = cmdline[index + i + 1];
                int index = -1;
                int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                else
                {
                    TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
                }

            }

            index++;
            view |= mainPage::View_Pressure;
        }/*else if(cmdline.at(index) == "-d"){ //distance sensor data, unit mm
            index_temp = index + 1;

            while(index_temp < cmdline.size())
            {
                index_temp++;
            }

            data_length = index_temp - index - 1;
            for(int i=0; i<data_length; i++){
                this->cmdManager->sensor->stroke[i+1] = cmdline[index + i + 1].toInt();

                QString SensorName = "Travelsensor" + QString::number(i+1);
                QString SensorValue = cmdline[index + i + 1];
                int index = -1;
                int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
                if(rtn == 1)
                {
                    sensor_list[index].name = SensorName;
                    sensor_list[index].value = SensorValue;
                }
                else
                {
                    LOG(INFO) << SensorName.toStdString().c_str()  << "not found !";
                }

            }

            index++;
            view |= mainPage::View_Stroke;
        }*/else if(cmdline.at(index) == "-dip"){ //dip sensor data, uint degree
            for(int i=0; i<24; i++){
                this->cmdManager->sensor->dip_angle[i/3][i%3] = 0.0f ;
            }

            index_temp = index + 1;
            while(index_temp < cmdline.size() && exp.exactMatch(cmdline[index_temp])){
                index_temp++;
            }
            data_length = index_temp - index - 1;
            QString SensorValue;
            QString SensorValue_three[3];
            for(int i=0; i<data_length; i++){
                this->cmdManager->sensor->dip_angle[i/3][i%3] = cmdline[index + i + 1].toFloat();               
                if(i % 3 == 0)
                {
                    SensorValue += cmdline[index + i + 1];
                    SensorValue_three[0] = cmdline[index + i + 1];
                }
                if(i % 3 == 1)
                {
                    SensorValue += " " + cmdline[index + i + 1];
                    SensorValue_three[1] = cmdline[index + i + 1];
                }
                if(i % 3 == 2)
                {
                    SensorValue += " " + cmdline[index + i + 1];
                    SensorValue_three[2] = cmdline[index + i + 1];
                }
                if(i % 3 == 2)
                {
                    QString SensorName = "Inclinationsensor" + QString::number(i/3+1);
                    int index_sensor = -1;
                    int rtn = this->cmdManager->findSensorIndexByName(SensorName,index_sensor);
                    if(rtn == 1)
                    {
                        sensor_list[index_sensor].name = SensorName;
                        sensor_list[index_sensor].value = SensorValue;
                        sensor_list[index_sensor].dip_value[0] = SensorValue_three[0];
                        sensor_list[index_sensor].dip_value[1] = SensorValue_three[1];
                        sensor_list[index_sensor].dip_value[2] = SensorValue_three[2];
                        SensorValue.clear();
                    }
                    else
                    {
                        TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
                    }
                }
            }
            index++;
            view |= mainPage::View_Dip_Angle;


        }else if(cmdline.at(index) == "-i") { // infra red data, 0 presents false, 1 presents true
            this->cmdManager->sensor->infra_red = cmdline[++index].toInt();

            QString SensorName = "Infraredsensor" + QString::number(1);
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }

        }/*else if(cmdline.at(index) == "-pos") { // shearer postion data, unit #

            this->cmdManager->sensor->shearer_position = cmdline[++index].toInt();

            LOG(INFO) << "========================================!";

            view |= mainPage::View_Shearer_Position;
        }*/else if(cmdline.at(index) == "-h"){ //height data, uint mm
            this->cmdManager->sensor->height = cmdline[++index].toInt();
            if(cmdManager->sensor->height)
                this->cmdManager->sensor->calc_hight_flag = true;
            else
                this->cmdManager->sensor->calc_hight_flag = false;
            QString SensorName = "HighSensor";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }

            view |= mainPage::View_Height;
        }else if(cmdline.at(index) == "-pa"){ //pitch_angle data
            this->cmdManager->sensor->pitch_angle = cmdline[++index].toFloat();

            QString SensorName = "Pitchanglesensor";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }

            view |= mainPage::View_PITCH_ANGLE;
        }else if(cmdline.at(index) == "-ra"){ //roll_angle data
            this->cmdManager->sensor->roll_angle = cmdline[++index].toFloat();

            QString SensorName = "Rollanglesensor";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }

            view |= mainPage::View_ROLL_ANGLE;
        }else if(cmdline.at(index) == "-t"){ //temperature data, uint degree
            this->cmdManager->sensor->temperature = cmdline[++index].toFloat();

            QString SensorName = "TemperatureSensor" + QString::number(1);
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_Temperature;
        }else if(cmdline.at(index) == "-hu"){ //humidity data, uint %
            this->cmdManager->sensor->humidity = cmdline[++index].toFloat();
            QString SensorName = "Humiditysensor" + QString::number(1);
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }

            view |= mainPage::View_Humidity;
        }else if(cmdline.at(index) == "-cv"){ //connect volt
            ++index;
            QString SensorName = "ConnectTotalVolt";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        }else if(cmdline.at(index) == "-cr"){ //connect cur
            ++index;
            QString SensorName = "ConnectTotalCur";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        }else if(cmdline.at(index) == "-dv1"){ //connect cur
            ++index;
            QString SensorName = "DriverVolt";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        }else if(cmdline.at(index) == "-dr1"){ //connect cur
            ++index;
            QString SensorName = "DriverCur";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        }else if(cmdline.at(index) == "-dv2"){ //connect cur
            ++index;
            QString SensorName = "DriverVolt2";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        }else if(cmdline.at(index) == "-dr2"){ //connect cur
            ++index;
            QString SensorName = "DriverCur2";
            QString SensorValue = cmdline[index];
            int index = -1;
            int rtn = this->cmdManager->findSensorIndexByName(SensorName,index);
            if(rtn == 1)
            {
                sensor_list[index].name = SensorName;
                sensor_list[index].value = SensorValue;
            }
            else
            {
                TLOG_INFO("%s not found !", SensorName.toStdString().c_str());
            }
            view |= mainPage::View_All;
        } else if (cmdline.at(index) == "-hl"){
            ++index;
            QStringList args;
            for (int j = 0; j < cmdline.size() - 2; j++) {
                args << cmdline.at(index + j);
            }

            QJsonObject param_obj;
            param_obj["humanLock"] = args.join(':');

            QJsonDocument doc(param_obj);
            QString str_send = doc.toJson(QJsonDocument::Compact);

            ParaSynchronizeThread* pst = ParaSynchronizeThread::getInstance();
            pst->send_para_synchronization_msg(str_send);
            view |= mainPage::View_All;
        } else{
            index++;
        }
    }

    emit this->cmdManager->signal_update_sensor_data(view);  //update sensor interface
}

void cmd_select::func(const QStringList& cmdline){
    int index = 1;
    int index_temp;
    int data_length = 0;
    unsigned int view = 0;
    while(index < cmdline.size()){
        if(cmdline.at(index) == "-r"){
            index_temp = index + 1;
            while(index_temp < cmdline.size() && !(cmdline[index_temp].startsWith('-'))){
                index_temp++;
            }
            data_length = index_temp - index - 1;
            if(data_length == 1){
                //uint8_t
                this->cmdManager->address_set->set_destination_address(static_cast<uint8_t>(cmdline[index+1].toUInt()));
            }else if(data_length == 2){
                this->cmdManager->address_set->set_destination_address(static_cast<uint8_t>(cmdline[index+1].toUInt()),
                        static_cast<uint8_t>(cmdline[index+2].toUInt()));
            }
            index++;
        }else if(cmdline.at(index) == "-d"){
            if(cmdline[++index] == "left_to_right"){
                this->cmdManager->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_LEFT_TO_RIGHT);
            }else{  //right_to_left
                this->cmdManager->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_RIGHT_TO_LEFT);
            }
            index++;
        }else{

            if(cmdline.size() == 4 && index == 3)
            {
                this->cmdManager->address_set->set_action_exe_direction(AddressSet::ACTION_EXE_DIR_GROUP);
                TLOG_INFO("1112--1112\n");
            }
            index++;
        }
    }
    view |= mainPage::View_Controlled_Address;
    emit this->cmdManager->signal_update_action_data(view);

}

void cmd_parameter::func(const QStringList& cmdline){
    int index = 1;
    QString str_category = "control_para";
    QString tab_name = "backwashing";
    QString parameter_name = "exec_position";
    QString str_label = "real_value";
    QString str_temp;
    while(index < cmdline.size()){
        if (cmdline.at(index) == "-set_pos") {
            str_temp = cmdline.at(++index);
            this->cmdManager->para->update("parameters.xml", str_category, tab_name, parameter_name, str_label, str_temp);
            this->cmdManager->lock_ui_off();
        } else if (cmdline.at(index) == "-cm") {
            str_temp = cmdline.at(++index);
            tab_name = "coal_drawing";
            parameter_name = "mode";

            //修改于7.29
            QJsonObject mode_obj;
            mode_obj[parameter_name] = str_temp;

            QJsonObject coal_obj;
            coal_obj[tab_name] = mode_obj;

            QJsonObject param_obj;
            param_obj[str_category] = coal_obj;

            QJsonDocument doc(param_obj);
            QString str_send = doc.toJson(QJsonDocument::Compact);

            ParaSynchronizeThread* pst = ParaSynchronizeThread::getInstance();                
            pst->send_para_synchronization_msg(str_send);
            //this->cmdManager->para->update("parameters.xml", str_category, tab_name, parameter_name, str_label, str_temp);
            this->cmdManager->lock_ui_off();
        } else {
            ++index;
        }
    }
    this->cmdManager->lock_ui_off();
}

int CmdManager::StateCheck(QStringList cmdline)
{
    if(this->IsInUrGent())
    {
        if((cmdline[0] == "key")||(cmdline[0] == "action"))
        {
            QString info = "hmi panic in lock or stop send a action cmd: ";
            for(int i = 0; i< cmdline.size();i++)
            {
                info += cmdline[i];
            }

            TLOG_WARN("%s", info.toStdString().c_str());
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void cmd_action::func(const QStringList& cmdline){
    int index = 1;
    unsigned int view = 0;

    while(index < cmdline.size()){
        if(cmdline.at(index) == "action_info"){
            index++;
            view |= mainPage::View_Action_Name;
            for(int i = 2; i < cmdline.size();i+=2)
            {
                if (!cmdManager->action_register->action_name_map.contains(cmdline[i])){//不包含就跳过
                    continue;
                } else {
                    this->cmdManager->address_set->action_id_str += cmdline[i] + "&" + cmdline[i+1];
                    if(i < (cmdline.size() - 2)){
                        this->cmdManager->address_set->action_id_str += "&";
                    }
                }
            }

            if(cmdline[2] == "PUSH_RAIL")
            {
                if(this->cmdManager->ss->get_system_state() == SystemState::Normal)//本架推溜
                {
                    this->cmdManager->ss->set_system_state(SystemState::Self_pull);
                    emit this->cmdManager->self_push_pull(2, QString::number(cmdline[2].toInt()/100));
                    this->cmdManager->self_push = 1;
                } else if(this->cmdManager->ss->get_system_state() == SystemState::LocalReal) {
                    this->cmdManager->lock_ui_off();
                }
            }else if (cmdManager->action_register->action_name_map.find(cmdline[2]).value().support_controled_type&0x03) {
                if(this->cmdManager->ss->get_system_state() == SystemState::Normal)//本架动作
                {
                    this->cmdManager->ss->set_system_state(SystemState::SelfControlled);
                    emit this->cmdManager->self_push_pull(3, "none");
                } else if(this->cmdManager->ss->get_system_state() == SystemState::LocalReal) {
                    this->cmdManager->lock_ui_off();
                }
            }
        }else if(cmdline.at(index) == "stop"){
            index++;
            this->cmdManager->address_set->action_id_str = "";
            view |= mainPage::View_Action_Name;
            if(this->cmdManager->ss->get_system_state() != SystemState::Controlled)//本架推溜
            {
                 if((this->cmdManager->ss->get_system_state() == SystemState::Self_pull) \
                         || (this->cmdManager->ss->get_system_state() == SystemState::LocalReal))
                 {
                     TLOG_INFO("system_state is %d", this->cmdManager->ss->get_system_state());
                 }
                 else
                 {
                     TLOG_INFO("system_state is not Self_pull");
                     this->cmdManager->lock_ui_off();
                     return;
                 }
            }
        }else if(cmdline.at(index) == "-auxiliary"){
            index++;
            if(this->cmdManager->action_register->action_aditional_list.size() > 0)//当有附加动作时
            {
                emit this->cmdManager->toShowauxiliary();
            }
            return;
        }else if(cmdline.at(index) == "-fs"){
            index++;
            if(cmdline[index] == "stop"){
                this->cmdManager->address_set->follow_dir = 0;
                index++;
            }else if(cmdline[index] == "up"){
                this->cmdManager->address_set->follow_dir = 1;
                index++;
            }else if(cmdline[index] == "down"){
                this->cmdManager->address_set->follow_dir = 2;
                index++;
            }
            view |= mainPage::View_Follow_Dir;
            this->cmdManager->lock_ui_off();
        }else if(cmdline.at(index) == "-ft"){
            index++;
            if(cmdline[index] == "STEP_INIT"){
                this->cmdManager->address_set->follow_step = "初始";
                index++;
            }else if(cmdline[index] == "STEP_STOP"){
                this->cmdManager->address_set->follow_step = "初始";
                index++;
            }else if(cmdline[index] == "STEP_UP_1"){
                this->cmdManager->address_set->follow_step = "上行1";
                index++;
            }else if(cmdline[index] == "STEP_UP_2"){
                this->cmdManager->address_set->follow_step = "上行2";
                index++;
            }else if(cmdline[index] == "STEP_UP_3"){
                this->cmdManager->address_set->follow_step = "上行3";
                index++;
            }else if(cmdline[index] == "STEP_UP_4"){
                this->cmdManager->address_set->follow_step = "上行4";
                index++;
            }else if(cmdline[index] == "STEP_DOWN_1"){
                this->cmdManager->address_set->follow_step = "下行1";
                index++;
            }else if(cmdline[index] == "STEP_DOWN_2"){
                this->cmdManager->address_set->follow_step = "下行2";
                index++;
            }else if(cmdline[index] == "STEP_DOWN_3"){
                this->cmdManager->address_set->follow_step = "下行3";
                index++;
            }else if(cmdline[index] == "STEP_DOWN_4"){
                this->cmdManager->address_set->follow_step = "下行4";
                index++;
            }
            view |= mainPage::View_Follow_Step;
            this->cmdManager->lock_ui_off();
        }else{
            index++;
        }
    }
    emit this->cmdManager->signal_update_action_data(view);
}

void cmd_lock::func(const QStringList& cmdline){
    int index = 1; //skip the "lock" string

    while(index < cmdline.size()){
        if(cmdline.at(index) == "-e"){
            index++;
        }else if(cmdline.at(index) == "-d"){
            index++;
        }else if(cmdline.at(index) == "-he"){
            index++;
        }else{
            index++;
        }
    }
}

void cmd_stop::func(const QStringList& cmdline){
    int index = 1;  //skip the "stop" string
    TLOG_INFO("cmd_stop func");
    while(index < cmdline.size()){
        if(cmdline.at(index) == "-e"){
            index++;
        }else if(cmdline.at(index) == "-d"){
            index++;
        }else{
            index++;
        }
    }
}

void cmd_config::func(const QStringList &cmdline){
    int index = 1;
    unsigned int view = 0;
    QString file = "parameters.xml";
    const QString category = "Template";
    const QString subblock = "one_key_coal";
    QString item;
    const QString value = "real_value";

    while(index < cmdline.size()){
        if(cmdline.at(index) == "-l"){ //pressure sensor data, unit MPa
            index++;
            this->cmdManager->address_set->set_local_address(static_cast<uint8_t>(cmdline[index++].toUInt()));
            view |= mainPage::View_Local_Address;
        }else if(cmdline.at(index) == "-d"){
            index++;
            this->cmdManager->sensor->tm_year = cmdline[index++].toInt();
            this->cmdManager->sensor->tm_mon  = cmdline[index++].toInt();
            this->cmdManager->sensor->tm_mday = cmdline[index++].toInt();
            this->cmdManager->sensor->tm_hour = cmdline[index++].toInt();
            this->cmdManager->sensor->tm_min  = cmdline[index++].toInt();
            this->cmdManager->sensor->tm_sec  = cmdline[index++].toInt();

            view |= mainPage::View_System_Date;
        }else if(cmdline.at(index) == "-t"){ //net phone
            index++;
            view |= mainPage::View_System_Date;
            //break;

           if(cmdline.at(index) == "start"){
                emit this->cmdManager->signal_show_net_phone(1);
                g_audio_msg.push_back("start");
                g_sem_audio.release();
            }else if(cmdline.at(index) == "end"){
                emit this->cmdManager->signal_show_net_phone(0);
                g_audio_msg.push_back("end");
                g_sem_audio.release();
            }
            view |= mainPage::View_System_Date;
        }else if(cmdline.at(index) == "-m"){
            index++;
            if(cmdline.size() == 3)
            {
                if(cmdline[2] == "start")
                {
                    emit this->cmdManager->signal_show_memory_coal(1);
                }
                else if(cmdline[2] == "end")
                {
                    //当还在计时 只记录了开始没有记录结束时
                    if(this->cmdManager->action_register->coal_map[this->cmdManager->current_step].is_start_timer == true)
                    {
                        int ms = static_cast<int>(this->cmdManager->coal_timer.elapsed());
                        Template_sheet sheet;
                        sheet.id = this->cmdManager->current_step.toInt();
                        double time_interval = static_cast<double>(ms / 1000.0);
                        sheet.time = QString::number(time_interval,'f',1);
                        sheet.name = this->cmdManager->action_register->coal_map.value(this->cmdManager->current_step).action_eng;
                        this->cmdManager->list_template.push_back(sheet);
                    }
                    else
                    {
                        int ms = static_cast<int>(this->cmdManager->sleep_timer.elapsed());
                        Template_sheet sheet;
                        sheet.id = 255;
                        double time_interval = static_cast<double>(ms / 1000);
                        sheet.time = QString::number(time_interval,'f',1);
                        sheet.name = "sleep";
                        this->cmdManager->list_template.push_back(sheet);
                    }

                    this->cmdManager->RecordMemory_coal();
                    this->cmdManager->coal_flag = 1;//关闭记忆放煤窗口标志
                }
            }
            else if(cmdline.size() == 5)
            {
                if(cmdline[2] == "id")
                {
                    if (cmdline[4] == "start")
                    {
                        if(this->cmdManager->action_register->coal_map.contains(cmdline[3]))
                        {
                           //按记忆放煤或每次动作开始和结束的时间
                           int ms = static_cast<int>(this->cmdManager->sleep_timer.elapsed());
                           //int ms = this->cmdManager->sleep_timer.();
                           Template_sheet sheet;
                           sheet.id = 255;
                           double time_interval = static_cast<double>(ms / 1000);
                           //QString str= QString::number(time_interval,'f',1);
                           sheet.time = QString::number(time_interval,'f',1);
                           //sheet.time=( (float)( (int)( (sheet.time+0.0005)*1000 ) ) )  /1000;
                           //str.setNum(sheet.time,'f',2);
                           sheet.name = "sleep";
                           this->cmdManager->list_template.push_back(sheet);

                           this->cmdManager->current_step = cmdline[3];
                           this->cmdManager->ss->set_system_state(SystemState::Coal_Put);
                           this->cmdManager->action_register->coal_map[cmdline[3]].is_start_timer = true;
                           this->cmdManager->coal_timer.restart();
                        }

                    }
                    else if (cmdline[4] == "end")
                    {
                        if(this->cmdManager->action_register->coal_map.contains(cmdline[3]))
                        {
                           this->cmdManager->action_register->coal_map[cmdline[3]].is_start_timer = false;
                           int ms = static_cast<int>(this->cmdManager->coal_timer.elapsed());
                           this->cmdManager->sleep_timer.start();
                           Template_sheet sheet;
                           sheet.id = cmdline[3].toInt();
                           double time_interval = static_cast<double>(ms / 1000);
                           sheet.time = QString::number(time_interval,'f',1);
                           sheet.name = this->cmdManager->action_register->coal_map.value(cmdline[3]).action_eng;
                           this->cmdManager->list_template.push_back(sheet);
                        }
                    }
                }
            }
        }else if(cmdline.at(index) == "-mpara"){
            index = cmdline.size();
            this->cmdManager->ss->set_system_state(SystemState::Coal_Put);
            QStringList para_list = cmdline;
            emit this->cmdManager->change_mem_coal_para(para_list);
            this->cmdManager->coal_flag = 1;
        }else if(cmdline.at(index) == "-update"){
            index++;
            if(cmdline.at(index) == "-rate"){
                index++;
                int progress = cmdline.at(index).toInt();
                emit this->cmdManager->signal_update_progress(progress);
                return;
            }
            if(cmdline.at(index) == "-failed"){

                emit this->cmdManager->signal_update_fail();
                return;
            }
        }else if(cmdline.at(index) == "-driSingle"){
            index++;
#if 0
            QList<QPair<QString, QString>> list;
            sub_str_to_list(cmdline.at(index), ':', '.', list);
            QVector<QString> info;
            int i;
            for(i=0; i<list.size(); i++)
            {
                TLOG_INFO("%s %s\n", list[i].first.toUtf8().data(), list[i].second.toUtf8().data());
                info.clear();
                info.push_back(list[i].first);
                sub_str_to_vector(list[i].second, ',', info);
                for(int j=0; j<info.size(); j++)
                    TLOG_INFO("%s\n", info[j].toUtf8().data());
                if(g_value_action_match.find(info[1]) == g_value_action_match.end())
                    info[1] = "无";
                else
                    info[1] = g_value_action_match.find(info[1]).value();
                update_driver_chn_config_xml(info);
            }
            driver_param_remote_sync *driver = driver_param_remote_sync::getInstance();
            //driver->load_xml(USED_XML_PATH, false);
#endif
            this->cmdManager->lock_ui_off();
        }else if(cmdline.at(index) == "-getfs"){
            index++;
            QString msg = "config -fs ";
            if(g_fault_status_flag == -1)
                g_fault_status_flag = 0;
            msg += QString::number(g_fault_status_flag);
            CommAcsSocketThread::getInstance()->send_socket_msg_to_acs(msg);
            this->cmdManager->lock_ui_off();
        }else{
            index++;
        }
    }
    emit this->cmdManager->signal_update_config_data(view);
}

bool cmd_system_fault::load_driver_xml(QString& file_path){
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("hmi:open driver xml file failed.");
        return false;
    }
    QDomDocument driver_xml;
    if(!driver_xml.setContent(&file))
    {
        file.close();
        TLOG_ERROR("hmi: Dom set driver xml content failed.");
        return false;
    }
    file.close();

    QDomElement root = driver_xml.documentElement(); //根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    if(node.isNull())
        return false;

    node = node.nextSibling();
    if(node.isNull())
        return false;
    QDomNode match_node = node.firstChild();
    while(!match_node.isNull())
    {
        m_driver_chn_config.push_back(match_node.toElement().text());
        match_node = match_node.nextSibling();
    }

    return true;
}

bool cmd_system_fault::load_fault_xml(QString& file_path){
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_ERROR("hmi:open fault xml file failed.");
        return false;
    }
    if(!this->fault_xml.setContent(&file))
    {
        file.close();
        TLOG_ERROR("hmi: Dom set fault xml content failed.");
        return false;
    }
    file.close();

    QDomElement root = this->fault_xml.documentElement(); //根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    if(node.isNull())
        return false;

    int fault_num = node.toElement().text().toInt();
    node = node.nextSibling();
    int pri_num = node.toElement().text().toInt();
    node = node.nextSibling();

    g_fault.resize(pri_num);
    int cur_pri;
    int cur_no = 0;
    int index = 0;
    QString fault_name;
    FAULT_PARAM_S fault_param;
    while(!node.isNull())
    {
        fault_name = node.toElement().attribute("name");

        QDomNode child_nod = node.firstChild();
        index = child_nod.toElement().text().toInt();
        child_nod = child_nod.nextSibling();
        cur_pri = child_nod.toElement().text().toInt();

        if(cur_pri > pri_num-1)
        {
            node = node.nextSibling();
            continue ;
        }
        if(++cur_no > fault_num)
            break ;
        child_nod = child_nod.nextSibling();
        fault_param.descriptor = child_nod.toElement().text();
        child_nod = child_nod.nextSibling();
        fault_param.dignosis = child_nod.toElement().text();
        fault_param.pri = cur_pri;
        fault_param.index = index;
        g_fault_param.insert(fault_name, fault_param);
        node = node.nextSibling();
    }
    return true;
}

bool cmd_system_fault::sub_str_to_map(QString input, char sign, QMap<QString, QString> &output)
{
    int size = input.length();
    bool flag = false;
    int out_size = 0;
    int pos = 0;
    QString first;
    QString second;
    for(int i=0; i<size; i++)
    {
        if((flag == false)&&(input[i] == sign))
            continue;
        if(flag == false)
        {
            flag = true;
            pos = i;
        }

        if(input[i] == sign)
        {
            if(first.length() == 0)
                first = input.mid(pos, i-pos);
            else
            {
                second = input.mid(pos, i-pos);
                second.toUtf8().data()[0] = 1;
                if((second.toUtf8().data()[0] == '0')||(second.toUtf8().data()[0] == '1'))
                {
                    if(++out_size > 2)
                        output.insert(first, second);
                    first.clear();
                }
                else
                {
                    QString result = "1";
                    if(++out_size > 2)
                        output.insert(first, result);
                    first = second;
                    second.clear();
                }
            }
            pos = i+1;
            if((pos == size)&&(first.length() != 0))
            {
                QString result = "1";
                if(++out_size > 2)
                    output.insert(first, result);
            }
        }
    }
    if((flag == true)&&(pos != size))
    {
        QString temp = input.mid(pos, size-pos);
        if(first.length() == 0)
        {
            if(++out_size > 2)
                output.insert(temp, "1");
        }
        else
        {
            second = temp;
            if((second.toUtf8().data()[0] == '0')||(second.toUtf8().data()[0] == '1'))
            {
                if(++out_size > 2)
                    output.insert(first, second);
                first.clear();
            }
            else
            {
                QString result = "1";
                if(++out_size > 2)
                    output.insert(first, result);
                first = second;
                if(++out_size > 2)
                    output.insert(first, result);
                second.clear();
            }
        }
    }
    return flag;
}

void cmd_system_fault::fault_handle(QString fault_data)
{
    QMap<QString, QString> fault;
    char *temp = strstr(fault_data.toUtf8().data(), "DriverChnFault");
    if(temp)
        fault.insert("DriverChnFault", temp+strlen("DriverChnFault")+1);
    else
        sub_str_to_map(fault_data, 32, fault);

    if(fault.isEmpty())
        return ;

    for(QMap<QString, QString>::iterator itera = fault.begin(); itera != fault.end(); ++itera)
    {
        if(g_port_visualization == 1)
            static_cast<port_visualization *>(CommAcsSocketThread::getInstance()->cmdManager->window)->update_status(itera.key(), itera.value());

        if(g_fault_param.contains(itera.key()) == false)
                continue;

        if((itera.value().toUtf8().data()[0] == '0')&&(itera.key().compare("DriverChnFault") != 0))
        {
            if(g_fault.at(g_fault_param.value(itera.key()).pri).contains(itera.key()) == false)
                continue;
            g_fault[g_fault_param.value(itera.key()).pri].remove(itera.key());
        }
        else
        {
            FAULT_INFO_S fault_info;
            fault_info.fault_diagnosis.clear();
            fault_info.fault_display = g_fault_param.value(itera.key()).descriptor;

            if(itera.key().compare("DriverChnFault") == 0)
            {
                if(itera.value().compare("00,00,00,00,00,00,00,00") == 0)
                {
                    if(g_fault.at(g_fault_param.value(itera.key()).pri).contains(itera.key()) == false)
                        continue;
                    g_fault[g_fault_param.value(itera.key()).pri].remove(itera.key());
                    return ;
                }
                QStringList list = itera.value().split(",");
                if(list.size() < 8)
                    return;

                uint sbc_1, sbc_2;
                bool ok;

                sbc_1 = (list.at(0).toInt(&ok, 16)&0xFF)|((list.at(1).toUInt(&ok, 16)&0xFF)<<8)|((list.at(2).toUInt(&ok, 16)&0xFF)<<16)|((list.at(3).toUInt(&ok, 16)&0xFF)<<24);
                sbc_2 = (list.at(4).toInt(&ok, 16)&0xFF)|((list.at(5).toUInt(&ok, 16)&0xFF)<<8)|((list.at(6).toUInt(&ok, 16)&0xFF)<<16)|((list.at(7).toUInt(&ok, 16)&0xFF)<<24);

                fault_info.fault_diagnosis += "s1:";
                if(m_driver_chn_config.size() < 28)
                    return ;
                for (int i=0; i<32; i++) {
                    if(((sbc_1>>i)&0x01) == 1)
                    {
                        fault_info.fault_diagnosis += m_driver_chn_config.at(i);
                        fault_info.fault_diagnosis += " ";
                    }
                }
                fault_info.fault_diagnosis += "; s2:";
                for (int i=0; i<32; i++) {
                    if(((sbc_2>>i)&0x01) == 1)
                    {
                        fault_info.fault_diagnosis += m_driver_chn_config.at(i);
                        fault_info.fault_diagnosis += " ";
                    }
                }
                fault_info.fault_diagnosis += ".\n";
                fault_info.fault_diagnosis += itera.value();
                fault_info.fault_diagnosis += "\n";
            }


            fault_info.fault_diagnosis += g_fault_param.value(itera.key()).dignosis;
            fault_info.fault_time = time(nullptr);
            if(g_fault.at(g_fault_param.value(itera.key()).pri).contains(itera.key()) == true)
                g_fault[g_fault_param.value(itera.key()).pri].remove(itera.key());
            g_fault[g_fault_param.value(itera.key()).pri].insert(itera.key(), fault_info);

        }
    }
    int fault_num = 0;
    for (int i=0; i<g_fault.size(); i++) {
        if(g_fault.at(i).size() == 0)
            continue;
        ++fault_num;
    }
    //DBG_ERR("%d\n", fault_num);
    if(fault_num == 0){
        if(g_fault_status_flag == 0)
            return ;
        g_fault_status_flag = 0;
        QString msg = "config -fs 0";
        CommAcsSocketThread::getInstance()->send_socket_msg_to_acs(msg);
        emit CommAcsSocketThread::getInstance()->inform_fault_status(0);
    }else {
        if(g_fault_status_flag == 1)
            return ;
        g_fault_status_flag = 1;
        QString msg = "config -fs 1";
        CommAcsSocketThread::getInstance()->send_socket_msg_to_acs(msg);
        emit CommAcsSocketThread::getInstance()->inform_fault_status(1);
    }

}
void cmd_system_fault::func(const QStringList &cmdline){
    int index = 2;
    struct error_msg msg;
    QString fault_data;
    fault_data = "system_fault -acs";
    while(index < cmdline.size()){
        fault_data += " ";
        fault_data += cmdline.at(index++);
    }

    fault_handle(fault_data);
    this->cmdManager->lock_ui_off();
}

void cmd_addata::func(const QStringList &cmdline){
    this->cmdManager->lock_ui_off();
}


