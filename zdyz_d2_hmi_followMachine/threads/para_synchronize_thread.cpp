#include "para_synchronize_thread.h"
#include "./Resources/error_code.h"
#include <mutex>
#include "update_progress_page.h"
//#include "host.h"
#include "../Resources/mqtt_message.h"
#include "../Resources/auto_follow_config.h"
#include "can_param_sync.h"
#include "../window/mainpage.h"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define BUF_SIZE_SYN        1024
#define PARA_SYNC_UDP_PORT  3131
#define BACK_GROUND_LIGHT "/sys/class/backlight/rgb-backlight/brightness"

extern Gui_config guisetting;
extern int GroupVolume;
extern int video_frames;//视频帧率
extern int GroupVolumeDistance;//组播间隔
extern int g_coal_machine_fence;

extern char ip_client[16];
extern void param_opc_node_value_write(int type, string category_name, string tab_name, string parameter_name_method, QString &value);

namespace {

QString baudrateToCacheIndex(int baudrate)
{
    return (baudrate == 115200) ? QString("1") : QString("0");
}

bool shouldApplyGlobalSync(Parameters *para, const QJsonValue &support_type,
                           const QString &cat_name, const QString &tab_name, const QString &para_name)
{
    if (support_type.isNull())
        return true;

    QString global_label("global");
    int support_type_index = 0;
    QString local_support_type;
    para->get_support_type(local_support_type, support_type_index);

    QString cat = cat_name;
    QString tab = tab_name;
    QString name = para_name;
    const QString is_global = para->query(cat, tab, name, global_label);
    return !((is_global.contains("g_local")) && (support_type != local_support_type));
}

} // namespace

ParaSynchronizeThread* ParaSynchronizeThread::pst = nullptr;

ParaSynchronizeThread::ParaSynchronizeThread(QObject *parent):QObject(parent)
{
    this->para = Parameters::getInstance();

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->action_register = ActionRegister::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    this->gui_set = gui_setting::getInstance();
}

void ParaSynchronizeThread::send_para_synchronization_msg(const QString &msg){
    TLOG_WARN("sync param : %s", msg.toUtf8().data());

    int  support_type_index;
    QString support_type;
    this->para->get_support_type(support_type, support_type_index);

    const uint8_t dstMin = static_cast<uint8_t>(comm_acs_thread->cmdManager->m_min_address);
    const uint8_t dstMax = static_cast<uint8_t>(comm_acs_thread->cmdManager->m_max_address);

    QJsonObject rootObj;
    rootObj[QString("start_supptor")] = dstMin;
    rootObj[QString("end_supptor")] = dstMax;
    rootObj[QString("support_type")] = support_type;

    QJsonDocument paramDoc = QJsonDocument::fromJson(msg.toUtf8());
    if (paramDoc.isObject()) {
        rootObj[QString("param")] = paramDoc.object();
    } else {
        rootObj[QString("param")] = msg;
    }

    const QByteArray payload = QJsonDocument(rootObj).toJson(QJsonDocument::Compact);

    if (CanParamSync::useCanTransport()) {
        if (CanParamSync::instance()->sendPayload(payload, dstMin, dstMax)) {
        }
        TLOG_WARN("CAN param sync send failed, fallback to MQTT");
    }

    QJsonObject json_obj, param_obj;
    QJsonArray array;
    if (paramDoc.isObject()) {
        param_obj[QString("param")] = paramDoc.object();
    } else {
        param_obj[QString("param")] = msg;
    }
    param_obj[QString("support_type")] = support_type;

    array.append(0);
    json_obj[QString("direction")] = array;
    json_obj[QString("topic")] = TOPIC_PARAMETER_EXTERN_SET;
    json_obj[QString("message")] = param_obj;
    QJsonDocument jsonDoc(json_obj);
    QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
    comm_acs_thread->m_client->publish(QString(TOPIC_EXTERN_MSG), jsonString.toUtf8().data());
}

void ParaSynchronizeThread::parse_msg(const QByteArray &message)
{
    QJsonDocument document = QJsonDocument::fromJson(message);
    if (!document.isObject()) {
        TLOG_ERROR("JSON document is not an object ");
        return ;
    }

    QJsonObject jsonObject = document.object();
    QJsonValue param_value = jsonObject.value(QString("param"));
    QJsonValue support_type = jsonObject.value(QString("support_type"));
    QJsonObject obj_value, obj_sub, obj_category;

    if(!param_value.isNull()){
        QString str = jsonObject.value(QString("param")).toString();
        QString msg;
        QStringList list = str.trimmed().split(":");

        if(list[0] == "sync_passwd")//sync passwd
        {
            if(list.size() != 4)
                return;
            gui_set->write(list[1], list[2], list[3]);

            guisetting.user_passwd_en = gui_set->settings_app->value("/user_passwd/user").toString();
            guisetting.admin_passwd_en = gui_set->settings_app->value("/user_passwd/admin").toString();
            guisetting.dev_passwd_en = gui_set->settings_app->value("/user_passwd/dev").toString();

            this->para->Decrypt(guisetting.user_passwd_en, guisetting.user_passwd);
            this->para->Decrypt(guisetting.admin_passwd_en, guisetting.admin_passwd);
            this->para->Decrypt(guisetting.dev_passwd_en, guisetting.dev_passwd);

        } else if(list[0] == "action") {
            if(this->comm_acs_thread->cmdManager->is_self_action_update == true)
            {
                this->comm_acs_thread->cmdManager->is_self_action_update = false;
            }
            else
            {
                QPair<QString, Action_info> data_asy;
                int result = this->action_register->Find_Asy_action(list[1],data_asy);
                if(result == 0)
                {
                    QString id = "id";
                    QString str_path = HMI_FILES_PATH_BASE;
                    QString file = str_path;
                    file += "/depend_files/action.xml";
                    QString action_value = data_asy.second.action_eng;
                    QString action_key = list[2];
                    TLOG_INFO("%s %s", action_key.toLocal8Bit().data(), action_value.toLocal8Bit().data());
                    this->action_register->update(file,action_value,id,action_key);
                }
            }
        } else if(list[0] == "timesyn") {//时间同步
            QString cmd = "date -s ";
            QStringList time_list = list[1].trimmed().split("-");
            QString syntime = time_list[0] + "-" + time_list[1] + "-" + time_list[2] + " " + time_list[3] + ":" + time_list[4] + ":" + time_list[5];

            cmd += "\"";
            cmd += syntime;
            cmd += "\"";
            TLOG_INFO("time syn start! %s", cmd.toStdString().c_str());
            system(cmd.toStdString().c_str());
            system("/sbin/hwclock -w");
            sync();
            TLOG_INFO("time syn successfully! %s", cmd.toStdString().c_str());
        } else if(list[0] == "system"||list[0] == "control_para"||list[0] == "follow_para"||(list[0]=="healthy" && list[1] != "port_config")||list[0]=="upgradeSync") {
            QString label("real_value");
            QString cat_name = list[0];
            //int tab = list[1].toInt();
            QString tab_name = list[1];     //system_para
            QString para_name = list[2];    //GroupVolume
            QString new_value = list[3];
            if(!para_name.compare("GroupVolume")){
                GroupVolume = new_value.toInt();
            }else if (!para_name.compare("GroupVolumeDistance")) {
                GroupVolumeDistance = new_value.toInt();
                if(GroupVolumeDistance == 0)
                    GroupVolumeDistance = 1;
            }else if(!para_name.compare("back_ground_light")){
                QString cmd = QString("echo ") + new_value + QString(" > ") + QString(BACK_GROUND_LIGHT);
                system(cmd.toUtf8().data());
            }else if(!para_name.compare("coal_machine_enter")){
                g_coal_machine_fence = new_value.toInt();
            }

            if(!support_type.isNull()){
                QString _label("global");

                int  _support_type_index;
                QString _support_type;
                this->para->get_support_type(_support_type, _support_type_index);

                QString str_isglobal = this->para->query(cat_name, tab_name, para_name, _label);
                if((str_isglobal.contains("g_local")) && (support_type != _support_type)){
                    return ;
                }
            }

            this->para->update("parameters.xml", cat_name, tab_name, para_name, label, new_value);
            //QString tab_name = this->para->query_tab_name_by_index(cat_name, tab);
            //TLOG_INFO("%s %d %s %s %s", cat_name.toUtf8().data(), tab, tab_name.toUtf8().data(), para_name.toUtf8().data(), new_value.toUtf8().data());

            QJsonObject json, json_child;
            QJsonArray array;

            obj_value[list[2]] = static_cast<QString>(list[3]).toInt();
            obj_sub[tab_name] = obj_value;
            obj_category[list[0]] = obj_sub;
            json[MSG_PARAM_BASIC] = obj_category;

            QJsonDocument json_Doc(json);
            QString json_String = QString(json_Doc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), json_String.toLatin1());

            if(cat_name == "healthy" && para_name.contains("input_passwd_interval")){
                emit this->comm_acs_thread->cmdManager->signal_update_input_passwd_interval(new_value.toInt());
            }else if(!para_name.compare("hydraulic_supporter_type")){
                action_register->reload();
                emit this->comm_acs_thread->cmdManager->signal_update_sensor_data(mainPage::View_SUP_TYPE);
                json["reload"] = "action.xml";
                QJsonDocument jsonDoc(json);
                QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());
            }
        } else if(list[0]=="healthy" && list[1] == "port_config"){
            QString label("real_value");
            QString cat_name = list[0];
            QString tab_name = list[1];
            QString para_name = list[2];
            QString new_value = list[3];

            if(!support_type.isNull()){
                QString _label("global");

                int  _support_type_index;
                QString _support_type;
                this->para->get_support_type(_support_type, _support_type_index);

                QString str_isglobal = this->para->query(cat_name, tab_name, para_name, _label);
                if((str_isglobal.contains("g_local")) && (support_type != _support_type)){
                    return ;
                }
            }

            int value = this->para->query_element_index(cat_name, tab_name, para_name, new_value);
            if(value == -1)
            {
                return;
            }

            this->para->update_range_set("parameters.xml",cat_name, tab_name, para_name,label, new_value);
            QJsonObject obj_value1, json_port1, json1;
            obj_value1["mode"] = new_value;
            json_port1[para_name] = obj_value1;
            json1[tab_name] = json_port1;
            QJsonDocument jsonDoc = QJsonDocument(json1);
            QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
            this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());

        } else if(list[0] == "synTemplate") {//模板同步
           if(list[1] == "null")
               return;

           QString category_name = "Template";
           QString tab_name = "one_key_coal";
           QString parameter_name = "syzk_template";
           QString intervals_string_label = "intervals";
           QString id_list_string_label = "steps";
           this->para->update("parameters.xml",category_name, tab_name, parameter_name, intervals_string_label,list[1]);
           this->para->update("parameters.xml",category_name, tab_name, parameter_name, id_list_string_label,list[2]);
           QJsonObject json, json_child;
           json_child["action"] = list[1];
           json_child["time"] = list[2];
           json["one_key_coal"] = json_child;
           QJsonDocument jsonDoc(json);
           QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
           this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
        } else if (list[0] == "humanLock") {
            if(list.size() < 2)
                return;
            msg = "sensor -hl2";
            for(int j=1; j<list.size(); j++)
            {
                msg += " ";
                msg += list[j];
            }
            this->comm_acs_thread->send_socket_msg_to_acs(msg);
        } else if (list[0] == "other_config") {
            QString category_name = list[0];
            QString tab_name = list[1];
            QString parameter_name = list[2];
            QString real_value = "real_value";
            QString slogan = list[3];
            int ret = 0;
            if (!slogan.trimmed().isEmpty() && parameter_name == "propaganda_slogan") {
                ret = this->para->updateAndSave("parameters.xml",
                                          category_name,
                                          tab_name,
                                          parameter_name,
                                          real_value,
                                          slogan);
                if (ret == 0) emit sig_update_slogan(slogan);
            }
        } else {
           TLOG_INFO("other parameters  %s", list[0].toStdString().c_str());
        }
    }
}

void ParaSynchronizeThread::parse_msg1(const QByteArray &message)
{
    QJsonDocument document = QJsonDocument::fromJson(message);
    if (!document.isObject()) {
        TLOG_ERROR("JSON document is not an object ");
        return;
    }

    QJsonObject rootObj = document.object();

    int start_supptor = rootObj.value("start_supptor").toInt();
    int end_supptor   = rootObj.value("end_supptor").toInt();
    QJsonValue support_type = rootObj.value("support_type");

    QJsonValue paramVal = rootObj.value("param");
    if (!paramVal.isObject()) {
        TLOG_ERROR("param is not an object");
        return;
    }
    QJsonObject paramObj = paramVal.toObject();

    if (paramObj.contains("timesyn") && paramObj.value("timesyn").isString())
    {
        QString time_str = paramObj.value("timesyn").toString();
        QStringList time_list = time_str.split("-");

        QString syntime = time_list[0] + "-" + time_list[1] + "-" + time_list[2] + " " + time_list[3] + ":" + time_list[4] + ":" + time_list[5];
        QString cmd = "date -s ";
        cmd += "\"";
        cmd += syntime;
        cmd += "\"";

        TLOG_INFO("time syn start! %s", cmd.toStdString().c_str());
        system(cmd.toStdString().c_str());
        system("/sbin/hwclock -w");
        sync();
        TLOG_INFO("time syn successfully! %s", cmd.toStdString().c_str());

        return;
    }

    if (paramObj.contains("sync_passwd") && paramObj.value("sync_passwd").isObject())
    {
        QJsonObject sync_passwd_obj = paramObj.value("sync_passwd").toObject();

        QString group = sync_passwd_obj.value("group").toString();
        QString user  = sync_passwd_obj.value("user").toString();
        QString pwd   = sync_passwd_obj.value("pwd").toString();

        if (!group.isEmpty() && !user.isEmpty() && !pwd.isEmpty())
        {
            gui_set->write(group, user, pwd);

            // 重新读取加密密码
            guisetting.user_passwd_en  = gui_set->settings_app->value("/user_passwd/user").toString();
            guisetting.admin_passwd_en = gui_set->settings_app->value("/user_passwd/admin").toString();
            guisetting.dev_passwd_en   = gui_set->settings_app->value("/user_passwd/dev").toString();

            // 解密到明文内存
            this->para->Decrypt(guisetting.user_passwd_en,  guisetting.user_passwd);
            this->para->Decrypt(guisetting.admin_passwd_en, guisetting.admin_passwd);
            this->para->Decrypt(guisetting.dev_passwd_en,   guisetting.dev_passwd);
        }

        // 密码同步处理完成，直接返回
        return;
    }

    if (paramObj.contains("humanLock")) {
        QString data = paramObj["humanLock"].toString();
        QStringList list = data.split(':');

        if (list.isEmpty()) return;

        QString msg = "sensor -hl2";
        for (int j = 1; j < list.size(); j++) {
            msg += " ";
            msg += list[j];
        }

        this->comm_acs_thread->send_socket_msg_to_acs(msg);
        return;
    }

    for (const QString& catName : paramObj.keys())
    {
        QJsonValue catVal = paramObj.value(catName);
        if (!catVal.isObject()) continue;
        QJsonObject tabObj = catVal.toObject();

        for (const QString& tabName : tabObj.keys())
        {
            QJsonValue tabVal = tabObj.value(tabName);
            if (!tabVal.isObject()) continue;
            QJsonObject paraObj = tabVal.toObject();

            for (const QString& paraName : paraObj.keys())
            {
                QJsonValue rawVal = paraObj.value(paraName);
                QString paraValue = rawVal.toVariant().toString();

                if (catName == "system"
                    || catName == "control_para"
                    || catName == "follow_para"
                    || (catName == "healthy" && tabName != "port_config")
                    || catName == "upgradeSync")
                {
                    QString label("real_value");
                    QString cat_name  = catName;
                    QString tab_name  = tabName;
                    QString para_name = paraName;
                    QString new_value = paraValue;

                    if (para_name == "GroupVolume") {
                        GroupVolume = new_value.toInt();
                    } else if (para_name == "GroupVolumeDistance") {
                        GroupVolumeDistance = new_value.toInt();
                        if (GroupVolumeDistance == 0)
                            GroupVolumeDistance = 1;
                    } else if (para_name == "back_ground_light") {
                        QString cmd = QString("echo ") + new_value + " > " + QString(BACK_GROUND_LIGHT);
                        system(cmd.toUtf8().data());
                    } else if (para_name == "coal_machine_enter") {
                        g_coal_machine_fence = new_value.toInt();
                    }

                    if (!shouldApplyGlobalSync(this->para, support_type, cat_name, tab_name, para_name))
                    {
                        continue;
                    }

                    if (cat_name == "healthy" && tab_name == "system_set" && para_name == "server_addr") {
                        if (rawVal.isString())
                            new_value = rawVal.toString();
                        this->comm_acs_thread->cmdManager->m_route_address = new_value.toInt();
                    }

                    this->para->update("parameters.xml", cat_name, tab_name, para_name, label, new_value);

                    QJsonObject obj_value, obj_sub, obj_category;
//                    if (cat_name == "healthy" && tab_name == "system_set" && para_name == "server_addr")
//                        obj_value[para_name] = new_value;
//                    else
//                        obj_value[para_name] = new_value.toInt();
                    obj_value[para_name] = new_value.toInt();
                    obj_sub[tab_name] = obj_value;
                    obj_category[cat_name] = obj_sub;

                    QJsonDocument json_Doc(obj_category);
                    QString json_String = QString(json_Doc.toJson(QJsonDocument::Compact));
                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), json_String.toLatin1());

                    if (cat_name == "healthy" && para_name.contains("input_passwd_interval")) {
                        emit this->comm_acs_thread->cmdManager->signal_update_input_passwd_interval(new_value.toInt());
                    } else if (para_name == "hydraulic_supporter_type") {
                        action_register->reload();
                        emit this->comm_acs_thread->cmdManager->signal_update_sensor_data(mainPage::View_SUP_TYPE);
                        obj_category["reload"] = "action.xml";
                        QJsonDocument jsonDoc(obj_category);
                        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), jsonString.toLatin1());
                    }
                }
                else if (catName == "healthy" && tabName == "port_config")
                {
                    QString label("real_value");
                    QString cat_name  = catName;
                    QString tab_name  = tabName;
                    QString para_name = paraName;

                    if (!shouldApplyGlobalSync(this->para, support_type, cat_name, tab_name, para_name))
                        continue;

                    if (rawVal.isObject())
                    {
                        QJsonObject portObj = rawVal.toObject();
                        QJsonObject obj_value1;
                        bool updated = false;

                        if (portObj.contains("mode"))
                        {
                            QString mode_value = portObj["mode"].toString();
                            if (this->para->query_element_index(cat_name, tab_name, para_name, mode_value) != -1) {
                                this->para->update_range_set("parameters.xml", cat_name, tab_name, para_name, label, mode_value);
                                obj_value1["mode"] = mode_value;
                                updated = true;
                            }
                        }

                        if (portObj.contains("baudrate"))
                        {
                            int baudrate = portObj["baudrate"].toInt();
                            QString baudrate_label("baudrate_value");
                            QString baudrate_value = baudrateToCacheIndex(baudrate);
                            this->para->update("parameters.xml", cat_name, tab_name, para_name, baudrate_label, baudrate_value);
                            obj_value1["baudrate"] = baudrate;
                            updated = true;
                        }

                        if (!updated)
                            continue;

                        QJsonObject json_port1, json1, json_catName;
                        json_port1[para_name] = obj_value1;
                        json1[tab_name] = json_port1;
                        json_catName[cat_name] = json1;

                        QJsonDocument jsonDoc(json_catName);
                        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
                    }
                    else
                    {
                        QString new_value = paraValue;
                        if (this->para->query_element_index(cat_name, tab_name, para_name, new_value) == -1)
                            continue;

                        this->para->update_range_set("parameters.xml", cat_name, tab_name, para_name, label, new_value);

                        QJsonObject obj_value1, json_port1, json1, json_catname;
                        obj_value1["mode"] = new_value;
                        json_port1[para_name] = obj_value1;
                        json1[tab_name] = json_port1;
                        json_catname[cat_name] = json1;

                        QJsonDocument jsonDoc(json_catname);
                        QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
                        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), jsonString.toLatin1());
                    }
                }
                else if (catName == "Template" && tabName == "one_key_coal" && paraName == "syzk_template")
                {
                    // paraValue 现在是一个 JSON 对象字符串，先转对象
                    QJsonObject tplObj = QJsonDocument::fromJson(paraValue.toUtf8()).object();

                    QString intervals = tplObj.value("intervals").toString();
                    QString steps     = tplObj.value("steps").toString();

                    if (intervals == "null" || steps == "null")
                        return;

                    QString category_name    = catName;
                    QString tab_name         = tabName;
                    QString parameter_name   = paraName;
                    QString intervals_label  = "intervals";
                    QString steps_label      = "steps";

                    this->para->update("parameters.xml", category_name, tab_name, parameter_name, intervals_label, intervals);
                    this->para->update("parameters.xml", category_name, tab_name, parameter_name, steps_label, steps);

                    // 回发消息
                    QJsonObject json, json_child, json_syzk_template, json_catName;
                    json_child["action"] = intervals;
                    json_child["time"]   = steps;
                    json_syzk_template[paraName] = json_child;
                    json["one_key_coal"] = json_syzk_template;
                    json_catName[catName] = json;

                    QJsonDocument jsonDoc(json_catName);
                    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);

                    this->comm_acs_thread->send_socket_msg_to_acs(TOPIC_PARAMETER_SET, jsonString.toLatin1());
                }
                else if (catName == "other_config")
                {
                    QString category_name   = catName;
                    QString tab_name        = tabName;
                    QString parameter_name  = paraName;
                    QString real_value      = "real_value";
                    QString slogan          = paraValue;

                    int ret = 0;
                    if (!slogan.trimmed().isEmpty() && parameter_name == "propaganda_slogan") {
                        ret = this->para->updateAndSave("parameters.xml",
                            category_name,
                            tab_name,
                            parameter_name,
                            real_value,
                            slogan);
                        if (ret == 0)
                            emit sig_update_slogan(slogan);
                    }
                }
                else
                {
                    TLOG_INFO("other parameters %s", catName.toStdString().c_str());
                }
            }
        }
    }
}

void ParaSynchronizeThread::parse_follow_config_v2(const QByteArray &message)
{
    QJsonParseError err;
    QJsonDocument document = QJsonDocument::fromJson(message, &err);
    if (err.error != QJsonParseError::NoError || !document.isObject()) {
        TLOG_WARN("parse_follow_config_v2: invalid json %s",
                  err.errorString().toUtf8().data());
        return;
    }

    QJsonObject root = document.object();
    QJsonObject followConfig = root.value("follow_config").toObject();
    if (followConfig.isEmpty()) {
        // allow payload that is already the follow_config object
        if (root.contains("Stages") || root.contains("Type")) {
            followConfig = root;
        } else {
            TLOG_WARN("parse_follow_config_v2: follow_config missing");
            return;
        }
    }

    if (!AutoFollowConfig::getInstance()->saveFollowConfig(followConfig)) {
        TLOG_WARN("parse_follow_config_v2: save failed");
        return;
    }

    emit sig_update_follow_stages();
}

