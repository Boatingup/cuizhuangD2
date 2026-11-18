#include "action_register.h"
#include "error_code.h"
#include "./Resources/dbg_print.h"
#include "parameters.h"

#include <QDebug>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>

ActionRegister* ActionRegister::action_register = nullptr;
extern std::mutex action_mutex;
std::mutex action_mutex;

ActionRegister::ActionRegister()
{
    reload();
}

QString ActionRegister::get_support_type(){
    QString type = "none";
    QDomElement root = this->dom_doc.documentElement();
    if(!root.attribute("hydraulic_supporter_type").isNull()){
        type = root.attribute("hydraulic_supporter_type");
    }
    return type;
}

int ActionRegister::load_action_xml_file(QString& file_path){
    QFile file(file_path);
    std::lock_guard<std::mutex> lck(action_mutex);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("hmi:open action xml file failed.");
        return -1;
    }
    if(!this->dom_doc.setContent(&file))
    {
        file.close();
        TLOG_INFO("hmi: Dom set action xml content failed.");
        return -2;
    }
    file.close();
    return 0;
}

int ActionRegister::load_action_xml_file_local(QString& file_path){
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("hmi:open action xml file failed.");
        return -1;
    }
    if(!this->dom_doc.setContent(&file))
    {
        file.close();
        TLOG_INFO("hmi: Dom set action xml content failed.");
        return -2;
    }
    file.close();
    return 0;
}

void ActionRegister::find_gif_by_name(QString &key, QString &gif_info)
{
    std::lock_guard<std::mutex> lck(action_mutex);
    QHash <QString, Action_info>::iterator it = this->action_name_map.find(key);
    if(it == this->action_register->action_name_map.end())
        return;
    gif_info = it.value().gif_name;
    return ;
}

void ActionRegister::Find_action(QStringList &id_list,QString &action_name)
{
    std::lock_guard<std::mutex> lck(action_mutex);
    for(int i = 0; i < id_list.size()-1;i+=2)
    {
        DBG_ERR("%s\n", id_list[i].toUtf8().data());
        QHash <QString, Action_info>::iterator it = this->action_name_map.find(id_list[i]);
        if(it != this->action_register->action_name_map.end())
        {
            if(i > 0 && i <= (id_list.size() - 1)) {
                action_name += "||";
                action_name += it.value().action_name;
                if(id_list[i+1] != "none")
                    action_name += id_list[i+1];
            } else {
                action_name += it.value().action_name;
                if(id_list[i+1] != "none")
                    action_name += id_list[i+1];
            }
        } else {
            if(i > 0 && i <= (id_list.size() - 1)) {
                action_name += "||";
                action_name += "未知";
            } else {
                action_name += "预警";
            }
        }
    }
}

int ActionRegister::getActionByindex(int index,QString & action_key,QString & action_value)
{
    std::lock_guard<std::mutex> lck(action_mutex);
    if(index > this->action_list.size() + 1)
    {
        action_key = "none";
        action_value = "none";
        return -1;
    }

    QList<QPair<QString,Action_info>>::const_iterator it = this->action_list.constBegin();
    int i = 0;
    if(index == 0){
        action_key = it->first;
        action_value = it->second.action_name;
        return 0;
    } else {
        while (it != this->action_list.constEnd()) {
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

int ActionRegister::getActionByindex(int index,QString & action_key,QString & action_value,QString &action_eng)
{
    std::lock_guard<std::mutex> lck(action_mutex);
    if(index > this->action_list.size() + 1)
    {
        action_key = "none";
        action_value = "none";
        return -1;
    }

    QList<QPair<QString,Action_info>>::const_iterator it = this->action_list.constBegin();
    int i = 0;
    if(index == 0) {
        action_key = it->first;
        action_value = it->second.action_name;
        action_eng = it->second.action_eng;
        return 0;
    } else {
        while (it != this->action_list.constEnd()) {
            ++it;
            i++;
            if(i == index)
            {
                action_key = it->first;
                action_value = it->second.action_name;
                action_eng = it->second.action_eng;
                return 0;
            }

        }
    }
    return 0;
}

void ActionRegister::getGroupNeighbourActAditionalList()
{
    neighbour_action_aditional_list.clear();
    group_action_aditional_list.clear();
    interval_action_aditional_list.clear();
    private_action_aditional_list.clear();
    for(int i = 0; i < this->action_aditional_list.size(); i++)
    {
        QPair<QString, Action_info> pair;
        pair.first = action_aditional_list[i].first;
        pair.second = action_aditional_list[i].second;

        if((action_aditional_list[i].second.support_controled_type & 0x0C) != 0)
            neighbour_action_aditional_list.push_back(pair);

        if((action_aditional_list[i].second.support_controled_type & 0x40) != 0)
            group_action_aditional_list.push_back(pair);

        if((action_aditional_list[i].second.support_controled_type & 0x180) != 0)
            interval_action_aditional_list.push_back(pair);

        if((action_aditional_list[i].second.support_controled_type & 0x03) != 0)
            private_action_aditional_list.push_back(pair);
    }
}

void ActionRegister::reload()
{
    std::lock_guard<std::mutex> lck(action_mutex);
    dom_doc.clear();
    this->action_map.clear();
    this->action_name_map.clear();
    this->action_list.clear();
    this->action_aditional_list.clear();
#if 0
    QString str_path = HMI_FILES_PATH_BASE;
    QString file = str_path;
    file += "/depend_files/action.xml";

    load_action_xml_file_local(file);


    QString type_name;
    int type_index;
    Parameters::getInstance()->get_support_type(type_name, type_index);
    if(get_support_type() != type_name)
    {
        QString file_type = str_path + "/depend_files/action.xml_" + QString::number(type_index);
        if(access(file_type.toUtf8().data(), F_OK) == 0)
        {
            TLOG_ERROR("%s > %s", file_type.toUtf8().data(), file.toUtf8().data());
            unlink(file.toUtf8().data());
            //QFile::copy(file_type, file);
            symlink(file_type.toUtf8().data(), file.toUtf8().data());
            sync();

            load_action_xml_file_local(file);
        }
    }
#else

    QString type_name;
    int type_index;
    Parameters::getInstance()->get_support_type(type_name, type_index);
    QString file_type = QString(HMI_FILES_PATH_BASE) + "/depend_files/action.xml_" + QString::number(type_index);
    load_action_xml_file_local(file_type);
#endif
    read_local();
    getGroupNeighbourActAditionalList();

}

int ActionRegister::update(QString file_path, QString& actionname,QString& label, QString& new_value){
    QFile file(file_path);
    std::lock_guard<std::mutex> lck(action_mutex);
    if(this->action_map.contains(new_value))
    {
        return 1;
    }
    //std::lock_guard<std::mutex> lck(action_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("descriptor") != actionname && !node.isNull()){
        node = node.nextSibling();
    }
    QDomNode para = node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }

    if(!para.isNull()){
        QDomNode old_node = para.firstChild();
        para.firstChild().setNodeValue(new_value);
        TLOG_INFO("%s %s %d", new_value.toLocal8Bit().data(), __FILE__, __LINE__);
        QDomNode new_node = para.firstChild();
        node.replaceChild(new_node, old_node);
    }
    if(!file.open(QFile::WriteOnly | QFile::Truncate))
        return -1;

    QTextStream out_stream(&file);
    out_stream.setCodec("utf-8");
    this->dom_doc.save(out_stream, 4); //缩进4格
    file.flush();
    fsync(file.handle());
    file.close();
    return 0;
}

void ActionRegister::read_local(){
    QDomElement root = this->dom_doc.documentElement();
    QDomNodeList node_list = root.childNodes();
    QString key_id, key_eng;
    struct Action_info myinfo;
    for(int i=0; i<node_list.size(); i++){
        if(node_list.at(i).nodeName().compare("action"))
            continue;
        myinfo.action_name = node_list.at(i).toElement().attribute("name");
        myinfo.action_eng = node_list.at(i).toElement().attribute("descriptor");
        QDomNode element = node_list.at(i);
        QDomNodeList label_list = element.childNodes();
        //判断是数字型还是开关型参数
        myinfo.is_additional = false;
        for(int j=0; j<label_list.size(); j++){
            if(label_list.at(j).toElement().tagName() == "id"){
                key_id = label_list.at(j).toElement().text();
                myinfo.action_id = key_id;
            }else if(label_list.at(j).toElement().tagName() == "additional_action"){
                myinfo.is_additional = label_list.at(j).toElement().text().toInt();
            }else if(label_list.at(j).toElement().tagName() == "support_controled_type"){
                myinfo.support_controled_type = label_list.at(j).toElement().text().toInt();
            }else if(label_list.at(j).toElement().tagName() == "action_gif_name"){
                myinfo.gif_name = label_list.at(j).toElement().text();
            }else if(label_list.at(j).toElement().tagName() == "actionname"){
                key_eng = label_list.at(j).toElement().text();
                myinfo.key_eng_name = key_eng;
            }
        }

        //放煤动作ID
        if((myinfo.action_eng == "finishing_beam") || (myinfo.action_eng == "contract_plugboard") || (myinfo.action_eng == "stretch_plugboard") || (myinfo.action_eng == "tail_beam"))
        {
            this->coal_map.insert(key_id,myinfo);
        }
        if(myinfo.is_additional == 1)
        {
            QPair<QString, Action_info> pair;
            pair.first = key_id;
            pair.second = myinfo;
            this->action_aditional_list.push_back(pair);
        }
        //DBG_ERR("%s:%s:%d\n", key_id.toUtf8().data(), myinfo.action_name.toUtf8().data(), myinfo.support_controled_type);
        TLOG_INFO("%s : %s", key_id.toUtf8().data(), myinfo.action_name.toUtf8().data());
        this->action_map.insert(key_id, myinfo);
        this->action_name_map.insert(key_eng, myinfo);
        //DBG_ERR("%s:%s:%d\n", key_eng.toUtf8().data(), myinfo.action_name.toUtf8().data(), myinfo.support_controled_type);
        QPair<QString, Action_info> pair;
        pair.first = key_id;
        pair.second = myinfo;
        this->action_list.push_back(pair);
    }
}

int ActionRegister::Find_Asy_action(QString name,QPair<QString, Action_info> &data)
{
    int rtn = 1;
    std::lock_guard<std::mutex> lck(action_mutex);
    for(QList<QPair<QString, Action_info>>::const_iterator it = this->action_register->action_list.begin(); it != this->action_register->action_list.end(); ++it){
        //LOG(INFO) << "id: " << it.key().toLocal8Bit().data() << " desc: " << it.value().toLocal8Bit().data();
        if(it->second.action_eng == name)
        {
            data = *it;
            rtn = 0;
            break;
        }
    }
    return rtn;
}
