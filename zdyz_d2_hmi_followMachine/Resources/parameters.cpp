// author:  xufeng
// date:    2021.11.07

#include "parameters.h"
#include <QDebug>
#include <QList>
#include <unistd.h>
#include <QString>
#include <QDomNodeList>
#include "error_code.h"
#include <mutex>
#include "guiconfig.h"
#include "log_head.h"

std::mutex parameter_mutex;
std::mutex para_mutex_thread;
std::mutex template_mutex;

Parameters* Parameters::parameters = nullptr;

QList<sensor_info> sensor_list;
QList<sensor_info> ad_list;
QList<sensor_info> wireless_list;

extern QString syzk_user;
extern QStringList user_get;

int Parameters::query_for_menuconfig(QString& category_name,QList <mainPage_uigonfig> & menuconfig)
{
    QString str;
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        QString test0 = node.toElement().attribute("category");
        node = node.nextSibling();
    }

    //QList <mainPage_uigonfig> sencond_level_list;
    QDomNode tab_node = node.firstChild(); //   get the first tab node(sub block)
    int index = 0;
    while(!tab_node.isNull()){
        //QString test =  node.toElement().attribute("name");
        mainPage_uigonfig menu;
        QStringList userlist_access;
        QStringList userlist_write;
        menu.page = tab_node.toElement().attribute("name");
        menu.page_chn = tab_node.toElement().attribute("descriptor");
        QString users_access = tab_node.toElement().attribute("access");

        if(users_access.size() > 0)
        {
            userlist_access = users_access.trimmed().split('|', QString::SkipEmptyParts);
            if(HasPrority(userlist_access))
            {
                menu.access = 1;
            }
            else
            {
                menu.access = 0;
            }
        }
        else
        {
            menu.access = 1;
        }

        QString users_write = tab_node.toElement().attribute("write");
        if(users_write.size() > 0)
        {
            userlist_write = users_write.trimmed().split('|', QString::SkipEmptyParts);
            if(HasPrority(userlist_write))
            {
                menu.write_available = 1;
            }
            else {
                menu.write_available = 0;
            }
        }
        else
        {
            menu.write_available = 1;
        }

        menu.class_level = tab_node.toElement().attribute("class").toInt();
        menu.haschild = tab_node.toElement().attribute("haschild").toInt();
        menuconfig.push_back(menu);
        if(menu.haschild == 1)
        {
            QList <mainPage_uigonfig> sencond_level_list;
            QDomNode sub_node = tab_node.firstChild();
            while(!sub_node.isNull()){
                QString test1 = sub_node.toElement().attribute("name");
                mainPage_uigonfig menu;
                QStringList userlist_access;
                QStringList userlist_write;
                menu.page = sub_node.toElement().attribute("name");
                menu.page_chn = sub_node.toElement().attribute("descriptor");
                QString users_access = sub_node.toElement().attribute("access");
                if(users_access.size() > 0)
                {
                    userlist_access = users_access.trimmed().split('|', QString::SkipEmptyParts);
                    if(HasPrority(userlist_access))
                    {
                        menu.access = 1;
                    }
                    else
                    {
                        menu.access = 0;
                    }
                }
                else
                {
                    menu.access = 1;
                }

                QString users_write = sub_node.toElement().attribute("write");
                if(users_write.size() > 0)
                {
                    userlist_write = users_write.trimmed().split('|', QString::SkipEmptyParts);
                    if(HasPrority(userlist_write))
                    {
                        menu.write_available = 1;
                    }
                    else
                    {
                        menu.write_available = 0;
                    }
                }
                else
                {
                    menu.write_available = 1;
                }
                menu.class_level = sub_node.toElement().attribute("class").toInt();
                menu.haschild = sub_node.toElement().attribute("haschild").toInt();
                sencond_level_list.push_back(menu);
                sub_node = sub_node.nextSibling();

            }
            menuconfig[index].nextclass_page = sencond_level_list;
        }
        else
        {

        }
        index++;
        tab_node = tab_node.nextSibling();
    }
    return 0;
}


bool Parameters::HasPrority(QStringList &list) const
{
     if(list.size() > 0)
     {
         for(auto user : list)
         {
             if(user == syzk_user)
             {
                 return 1;
             }
             else
             {
                 //int k = 1;
                 continue;
             }
         }
         return 0;
     }
     else {
         return 1;
     }
}

QDomNode Parameters::query_subnode(QString& category_name, QString& tab_name, QString& parameter_name) const
{
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
        QString test21 =  node.toElement().attribute("category");
        //qDebug() << test21;
        TLOG_INFO("%s\n", test21.toUtf8().data());
    }

    QDomNode tab_node = node.firstChild();
    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
        QString test22 =  tab_node.toElement().attribute("name");
        TLOG_INFO("%s\n", test22.toUtf8().data());
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
        QString test23 =  node.toElement().attribute("name");
        TLOG_INFO("%s\n", test23.toUtf8().data());
    }

    return sub_node.firstChild();
}

//读出记忆模板
int Parameters::queryTemplate(QString & intervals_string,QString & id_list_string,QStringList &intervals_list,QStringList &id_list)
{
    QString category_name = "Template";
    QString tab_name = "one_key_coal";
    QString parameter_name = "syzk_template";
    QString intervals_label = "intervals";
    QString id_list_label = "steps";

    std::lock_guard<std::mutex> lck(template_mutex);

    intervals_string = query(category_name, tab_name, parameter_name, intervals_label);
    id_list_string = query(category_name, tab_name, parameter_name, id_list_label);

    if(intervals_string.size() > 0)
    {
        intervals_list = intervals_string.trimmed().split('|', QString::SkipEmptyParts);
        id_list = id_list_string.trimmed().split('|', QString::SkipEmptyParts);
        if(intervals_list.size() == intervals_list.size())
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}

//写入记忆模板
int Parameters::WriteTemplate(QList <Template_sheet> & list_template)
{
    QString category_name = "Template";
    QString tab_name = "one_key_coal";
    QString parameter_name = "syzk_template";
    QString intervals_string_label = "intervals";
    QString id_list_string_label = "steps";
    QString intervals_string ;
    QString id_list_string;

    std::lock_guard<std::mutex> lck(template_mutex);

    if(list_template.size()==0)
        return 1;

    for(int i = 0;i < list_template.size();i++)
    {
        if(i == 0)
        {
            intervals_string += list_template[i].time;
            id_list_string +=  QString::number(list_template[i].id);
        }
        else
        {
            intervals_string += "|";
            intervals_string += list_template[i].time;
            id_list_string += "|";
            id_list_string += QString::number(list_template[i].id);
        }
    }

    update("parameters.xml",category_name, tab_name, parameter_name, intervals_string_label,intervals_string);
    update("parameters.xml",category_name, tab_name, parameter_name, id_list_string_label,id_list_string);

    return 0;
}

Parameters::Parameters()
{
    //std::lock_guard<std::mutex> lck(parameter_mutex);
    QString file_name("/parameters.xml");

    QString str_path = HMI_FILES_PATH_BASE;
    QString file = str_path;
    file += file_name;

    load_parameter_file_local(file);

    this->category_name.insert("system", "系统参数");
    this->category_name.insert("control_para",  "控制参数");
    this->category_name.insert("follow_para", "跟机参数");
    this->category_name.insert("healthy", "系统设置");
    this->category_name.insert("upgradeSync", "升级同步");
    this->category_name.insert("set_para", "其他配置");

    this->category_index.insert("system", 0);
    this->category_index.insert("control_para", 1);
    this->category_index.insert("follow_para", 2);
    this->category_index.insert("healthy", 3);
    this->category_index.insert("upgradeSync", 4);
    this->category_index.insert("set_para", 5);


    QStringList list;
    list << "系统设置";
    list << "数据采集";
    list << "自动补压";
    list << "惯导找直";
    list << "防碰撞";
    this->tab_name << list;
    list.clear();

    list << "推溜";
    list << "移架";
    list << "护帮";
    list << "伸缩梁";
    list << "喷雾";
    list << "反冲洗";
    list << "后溜";
    list << "放煤";
    list << "自动闭锁";
    this->tab_name << list;
    list.clear();

    list << "跟机状态";
    list << "基本参数";
    list << "动作参数";
    list << "阶段显示";
    this->tab_name << list;
    list.clear();


    list << "模式配置";
    list << "传感器标定";
    list << "控制开关";
    list << "系统配置";
    list << "传感器数据";
    list << "无线信息";
    this->tab_name << list;
    list.clear();

    list << "参数配置";
    this->tab_name << list;
    list.clear();

    QString type_name;
    int type_index;
    get_support_type(type_name, type_index);
}

bool Parameters::get_support_type(QString & type_name, int & type_index)
{
    QString category_name = "system";
    QString tab_name = "system_para";
    QString parameter_name = "hydraulic_supporter_type";
    QString label = "real_value";
    type_name = query(category_name, tab_name, parameter_name, label);
    type_index = type_name.toInt();

    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    label = "range_set";
    QDomNode para = sub_node.firstChild();

    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }

    while(para.toElement().attribute("index").toInt() != type_index && !para.isNull()){
        para = para.nextSibling();
    }

    type_name = para.toElement().text().toUtf8().data();
    return true;
}

int Parameters::getAccess(QString& category_name,int tab,int index,QString &label_name)
{
    QString parameter_name = query_parameter_name(category_name, tab, index);
    QString access = label_name;
    QString users = query(category_name, tab, parameter_name, access);
    QStringList usr_list;
    if(users.size() > 0)
    {
        usr_list = users.trimmed().split('|', QString::SkipEmptyParts);
        //int k = 0;
        if(usr_list.size() > 0)
        {
            return HasPrority(usr_list);
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

int Parameters::getAccess(QString& category_name,QString& tab_name,QString& parameter_name,QString &label,QStringList &usr_list)
{
    QString users = query(category_name, tab_name, parameter_name, label);
    if(users.size() > 0)
    {
        usr_list = users.trimmed().split('|', QString::SkipEmptyParts);
        if(usr_list.size() > 0)
        {
            return HasPrority(usr_list);
        }
    }
    else {
        return 1;
    }
    return 0;
}

//return 0 if sucessed, -1 for opening file failure
int Parameters::load_parameter_file_local(QString& file_path){
    QFile file(file_path);
    //std::lock_guard<std::mutex> lck(parameter_mutex);
    dom_doc.clear();
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open parameter file failed. %s %d", __FILE__, __LINE__);
        return -1;
    }
    if(!this->dom_doc.setContent(&file))
    {
        file.close();
        TLOG_INFO("Dom set content failed. %s %d", __FILE__, __LINE__);
        return -2;
    }
    file.close();

    QString category_name = "healthy";
    QString tab_name = "SenSorData";
    query_subtomap_local(category_name,tab_name,sensor_map,sensor_list);

    tab_name = "AD";
    query_subtomap_local(category_name,tab_name,ad_map,ad_list);

    tab_name = "WirelessData";
    query_subtomap_local(category_name,tab_name,ad_map,wireless_list);

    return 0;
}
//return 0 if sucessed, -1 for opening file failure
int Parameters::load_parameter_file(QString& file_path){
    QFile file(file_path);
    std::lock_guard<std::mutex> lck(parameter_mutex);
    dom_doc.clear();
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open parameter file failed. %s %d", __FILE__, __LINE__);
        return -1;
    }
    if(!this->dom_doc.setContent(&file))
    {
        file.close();
        TLOG_INFO("Dom set content failed. %s %d", __FILE__, __LINE__);
        return -2;
    }

    file.close();


    QString category_name = "healthy";
    QString tab_name = "SenSorData";
    query_subtomap_local(category_name, tab_name, sensor_map, sensor_list);

    tab_name = "AD";
    query_subtomap_local(category_name, tab_name, ad_map, ad_list);

    tab_name = "WirelessData";
    query_subtomap_local(category_name, tab_name, ad_map, wireless_list);

    return 0;
}

QString Parameters::query_tab_name_by_index(QString& category_name, int tab) const{
    QString str = "none";

    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    TLOG_INFO("%s", root.nodeName().toLocal8Bit().data());

    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        node = node.nextSibling();
    }
    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);
    if(!tab_node.isNull())
        return tab_node.toElement().attribute("name");
    return str;
}

//要注意判断node或element是否为空
QString Parameters::query(QString& category_name, QString& tab_name, QString& parameter_name, QString& label) const{
    QString str;
    QDomNode para = query_subnode(category_name, tab_name, parameter_name);
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }
    if(!para.isNull()){
        QString str = para.toElement().text();
        TLOG_INFO("%s", str.toUtf8().data());
        return para.toElement().text();
    }

    return str;
}

QString Parameters::query_port_mode_by_id(QString& category_name, QString& tab_name, QString& parameter_name, QString &label, QString& index) const{
    QString str = "none";
    QDomNode para = query_subnode(category_name, tab_name, parameter_name);
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }

    while(para.toElement().attribute("index").compare(index) && !para.isNull()){
        para = para.nextSibling();
    }
    if(!para.isNull()){
        //LOG(INFO) << para.toElement().nodeName().toLocal8Bit().data() << para.toElement().text().toLocal8Bit().data();
        return para.toElement().attribute("name");
    }

    return str;
}

QString Parameters::query_subtomap_local(QString& category_name, QString& parameter_name, QHash <QString,int> &map_data,QList <sensor_info> &list_data) const{
    list_data.clear();
    map_data.clear();
    QString str;
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node;
    QDomNodeList tab_list = node.childNodes();
    for(int k = 0; k < tab_list.size();k++)
    {
        if(tab_list.at(k).toElement().attribute("name") == parameter_name)
        {
            tab_node = tab_list.at(k);
        }
    }

    QDomNodeList sensor_list = tab_node.childNodes();
    for(int i = 0;i < sensor_list.size();i++)
    {
        map_data.insert(sensor_list.at(i).toElement().attribute("name"),i);
    }

    for(int i = 0;i < sensor_list.size();i++)
    {
        struct sensor_info info;
        info.index = i;
        info.name = sensor_list.at(i).toElement().attribute("name");

        QDomNodeList label_list = sensor_list.at(i).toElement().childNodes();

        for(int j=0; j<label_list.size(); j++)
        {
            QDomNode para = label_list.at(j);
            if(para.toElement().tagName() == "range_low"){
                QString low = para.toElement().text();
                info.min = low.toFloat();
            }
            if(para.toElement().tagName() == "range_high"){
                QString high = para.toElement().text();
                info.max = high.toFloat();
            }
        }
        info.value = "none";   
        list_data.push_back(info);
    }

    return str;
}

//要注意判断node或element是否为空
QString Parameters::query_subtomap(QString& category_name, QString& parameter_name, QHash <QString,int> &map_data,QList <sensor_info> &list_data) const{
    std::lock_guard<std::mutex> lck(parameter_mutex);
    list_data.clear();
    map_data.clear();
    QString str;
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    TLOG_INFO("%s", root.nodeName().toLocal8Bit().data());
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node;
    QDomNodeList tab_list = node.childNodes();
    for(int k = 0; k < tab_list.size();k++)
    {
        if(tab_list.at(k).toElement().attribute("name") == parameter_name)
        {
            tab_node = tab_list.at(k);
        }
    }

    QDomNodeList sensor_list = tab_node.childNodes();
    for(int i = 0;i < sensor_list.size();i++)
    {
        map_data.insert(sensor_list.at(i).toElement().attribute("name"),i);
    }

    for(int i = 0;i < sensor_list.size();i++)
    {
        struct sensor_info info;
        info.index = i;
        info.name = sensor_list.at(i).toElement().attribute("name");
        info.value = "none";
        list_data.push_back(info);
    }
    return str;
}

//要注意判断node或element是否为空   111
QString Parameters::query(QString& category_name, int tab, QString& parameter_name, QString& label) const{
    QString str;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    //LOG(INFO)<<root.nodeName().toLocal8Bit().data();
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name&& !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }
    if(!para.isNull()){
        //LOG(INFO) << para.toElement().nodeName().toLocal8Bit().data() << para.toElement().text().toLocal8Bit().data();
        return para.toElement().text();
    }

    return str;
}

//要注意判断node或element是否为空
QString Parameters::query_parameter_name(QString& category_name, QString& tab_name, int index) const{
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    while(node.toElement().attribute("tab") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    for(int i = 0; i < index; i++){
        sub_node = sub_node.nextSibling();
    }
    return sub_node.toElement().attribute("name");
}


QString Parameters::query_parameter_name(QString& category_name, int tab, int index) const{

    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();

    }

    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);

    QDomNode sub_node = tab_node.firstChild();
    for(int i = 0; i < index; i++){
        sub_node = sub_node.nextSibling();
    }
    return sub_node.toElement().attribute("name");
}

QStringList Parameters::query_error_set(QString& category_name, QString& tab_name, QString& parameter_name,QMap <QString,int> &map_error) const{
    QStringList list;
    QDomNode para = query_subnode(category_name, tab_name, parameter_name);
    int index = 0;
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            list.append(para.toElement().text());
            map_error.insert(para.toElement().attribute("name"),index);
            index++;
        }
        para = para.nextSibling();
    }
    return list;
}

QStringList Parameters::query_range_set_index(QString& category_name, QString& tab_name, QString& parameter_name) const{
    QStringList list;
    QDomNode para = query_subnode(category_name, tab_name, parameter_name);
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            list.append(para.toElement().text());
            //TLOG_INFO("%s\n", para.toElement().attribute("index").toUtf8().data());
        }
        para = para.nextSibling();
    }
    return list;
}

/*********************************************************
 * @file      : parameters.cpp
 * @brief     :
 * @author    : fxs
 * @date      : 2026-06-02
 * @version   : V1.0
 *********************************************************/

PortModeInfo Parameters::queryPortModeInfoByIndex(QString category,int tab, QString parameter_name, int modeIndex) const
{
    PortModeInfo info;
    info.modeIndex = modeIndex;

    QDomElement root = dom_doc.documentElement();
    QDomNode node = root.firstChild();

    // 查找 category
    while (!node.isNull())
    {
        QDomElement e = node.toElement();
        if (!e.isNull() && e.attribute("category") == category)
            break;
        node = node.nextSibling();
    }

    if (node.isNull())
        return info;

    QDomNode subblock = node.childNodes().at(tab);
    QDomNodeList elements = subblock.childNodes();

    for (int i = 0; i < elements.size(); ++i)
    {
        QDomElement ele = elements.at(i).toElement();
        if (ele.isNull() || ele.attribute("name") != parameter_name)
            continue;

        // 开始解析当前 element
        QString baud_num;

        QDomNodeList childs = ele.childNodes();
        for (int j = 0; j < childs.size(); ++j)
        {
            QDomElement c = childs.at(j).toElement();
            if (c.isNull()) continue;

            if (c.tagName() == "baudrate_value")
            {
                baud_num = c.text();
                info.baudrateIndex = c.text().toInt();
            }
        }

        // 遍历 range_set，严格匹配 real_index
        for (int j = 0; j < childs.size(); ++j)
        {
            QDomElement rs = childs.at(j).toElement();
            if (rs.isNull() || rs.tagName() != "range_set")
                continue;

            // 只匹配 real_value 对应的 index
            if (rs.attribute("index").toInt() != modeIndex)
                continue;

            // 解析 text 和 baudrate
            QDomNodeList rsChild = rs.childNodes();
            bool hasText = false;
            for (int k = 0; k < rsChild.size(); ++k)
            {
                QDomElement item = rsChild.at(k).toElement();
                if (item.isNull()) continue;

                if (item.tagName() == "text")
                {
                    info.modeText = item.text();
                    hasText = true;
                }
                else if (item.tagName() == "baudrate")
                {
                    info.baudrateList << item.text();
                }
            }

            if (!hasText)
                info.modeText = rs.text();

            // 找到直接返回
            return info;
        }

        break;
    }

    return info;
}

/*********************************************************
 * @file      : parameters.cpp
 * @brief     :
 * @author    : fxs
 * @date      : 2026-06-09
 * @version   : V1.0
 *********************************************************/

PortModeInfo Parameters::queryPortModeInfo(QString category, int tab, QString parameter_name) const
{
    QString str = "real_value";
    QString realValue = query(category, tab, parameter_name, str);
    int modeIndex = realValue.toInt();
    return queryPortModeInfoByIndex(category, tab, parameter_name, modeIndex);
}

QStringList Parameters::query_range_set(QString& category_name, QString& tab_name, QString& parameter_name) const{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }

    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            list.append(para.toElement().text());
        }
        para = para.nextSibling();
    }
    return list;
}

QString Parameters::query_element_name(QString& category_name, QString& tab_name, QString& parameter_name, int index) const{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }

    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            if(index == 0)
                break;
            --index;
        }
        para = para.nextSibling();
    }
    return para.toElement().attribute("name");
}

int Parameters::query_element_index(QString& category_name, QString& tab_name, QString& parameter_name, QString& name) const{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    int index = -1;
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }

    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            if(para.toElement().attribute("name") == name){
                index = para.toElement().attribute("index").toInt();
                break;
            }
        }
        para = para.nextSibling();
    }
    return index;
}

QStringList Parameters::query_range_set(QString& category_name, int tab, QString& parameter_name) const{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);
    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }

    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != "end" && !para.isNull()){               
        if(para.toElement().tagName() == "range_set"){
            QDomElement rangeElem = para.toElement();
            QDomNode textNode = rangeElem.firstChildElement("text");
            QString displayText;

            if (!textNode.isNull()) {
                displayText = textNode.toElement().text();
            } else {
                displayText = rangeElem.text();
            }

            if (!displayText.isEmpty()) {
                list.append(displayText);
            }
        }
        para = para.nextSibling();
    }
    return list;
}


QStringList Parameters::query_range_set(QString& category_name, int tab, QString& parameter_name,QStringList &name_list) const{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);
    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }

    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != "end" && !para.isNull()){
        if(para.toElement().tagName() == "range_set"){
            list.append(para.toElement().text());
            name_list.append(para.toElement().attribute("name"));
        }
        para = para.nextSibling();
    }
    return list;
}


int Parameters::getBlockNumber(QString& category_name)
{
    QStringList list;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull() ){
        node = node.nextSibling();
    }

    return(node.childNodes().size());
}

//return 0 if sucessed
int Parameters::update_sync(QString file_path){
#if 0
    TLOG_INFO("update_sync [%s]\n", file_path.toUtf8().data());
    QString file_name;
    if(file_path.compare("parameters.xml") == 0){
        file_name = "/userdata/config/hmi_files/parameters.xml";
    } else {
        file_name = file_path;
    }

    QString file_back = file_name + QString("_back");
    if(access(file_back.toUtf8().data(), F_OK) == 0){
        unlink(file_back.toUtf8().data());
    }

    if(QFile::copy(file_name, file_back) == false){
        TLOG_INFO("copy failed errno:%d\n", errno);
        return -1;
    }

    QFile file(file_name);
    std::lock_guard<std::mutex> lck(parameter_mutex);

    if(!file.open(QFile::WriteOnly | QFile::Truncate)){
        TLOG_INFO("open failed:%d\n", errno);
        return -1;
    }

    QTextStream out_stream(&file);
    out_stream.setCodec("utf-8");
    this->dom_doc.save(out_stream, 4); //缩进4格
    bool result = file.flush();
    fsync(file.handle());
    file.close();

    if(!file.open(QFile::ReadOnly)){
        TLOG_INFO("open failed:%d\n", errno);
        return -1;
    }

    QByteArray data = file.readAll();
    bool contain_result = data.contains("accessManagemnet");
    file.close();
    if((result == false) || (contain_result == false)){
        TLOG_INFO("flush failed:%d [%d-%d]\n", errno, result, contain_result);
        unlink(file_name.toUtf8().data());
        QFile::copy(file_back, file_name);
        return -1;
    }

    system("sync");
#endif
    return 0;
}

//return 0 if sucessed
int Parameters::update_cache(QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_value){

    TLOG_INFO("update_cache %s %s %s %s", category_name.toUtf8().data(), tab_name.toUtf8().data(), parameter_name.toUtf8().data(), new_value.toUtf8().data());
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();

    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }
    if(!para.isNull()){
        QDomNode old_node = para.firstChild();
        para.firstChild().setNodeValue(new_value);
        QDomNode new_node = para.firstChild();
        sub_node.replaceChild(new_node, old_node);
    }

    return 0;
}

int Parameters::update_range_set(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& range_set_new_name){

    TLOG_WARN("%s %s %s %s %s %s", file_path.toUtf8().data(), category_name.toUtf8().data(), tab_name.toUtf8().data(), parameter_name.toUtf8().data(), label.toUtf8().data(), range_set_new_name.toUtf8().data());

    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    QDomNodeList tab_node_list = node.childNodes();

    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }

    QDomNode para1 = sub_node.firstChild();
    while((para1.toElement().tagName() != QString("range_set") || para1.toElement().attribute("name") != range_set_new_name) && !para.isNull()){
        para1 = para1.nextSibling();
    }

    if(!para.isNull() != para1.isNull()){
        QDomNode old_node = para.firstChild();
        para.firstChild().setNodeValue(para1.toElement().attribute("index"));
        TLOG_INFO("%s", para1.toElement().attribute("index").toLocal8Bit().data());
        QDomNode new_node = para.firstChild();
        sub_node.replaceChild(new_node, old_node);
    }
    return 0;
}

//return 0 if sucessed
int Parameters::update(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_value){
    TLOG_WARN("%s %s %s %s %s %s", file_path.toUtf8().data(), category_name.toUtf8().data(), tab_name.toUtf8().data(), parameter_name.toUtf8().data(), label.toUtf8().data(), new_value.toUtf8().data());

    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }

    QDomNode tab_node = node.firstChild();
    QDomNodeList tab_node_list = node.childNodes();

    while(tab_node.toElement().attribute("name") != tab_name && !tab_node.isNull()){
        tab_node = tab_node.nextSibling();
    }

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }
    if(!para.isNull()){
        QDomNode old_node = para.firstChild();
        QString old_value = old_node.nodeValue();
        if (old_value == new_value)
        {
            return 1;
        }

        para.firstChild().setNodeValue(new_value);
        QDomNode new_node = para.firstChild();
        TLOG_WARN("%s", new_node.nodeValue().toUtf8().data());
//        sub_node.replaceChild(new_node, old_node);
    }
    return 0;
}

/*********************************************************
 * @file      : parameters.cpp
 * @brief     :
 * @author    : fxs
 * @date      : 2026-07-07
 * @version   : V1.0
 *********************************************************/

int Parameters::updateAndSave(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_value)
{
    int ret = update(file_path, category_name, tab_name, parameter_name, label, new_value);
    if (ret != 0) return ret;

    QString file_name;
    if(file_path.compare("parameters.xml") == 0)
        file_name = "/userdata/config/hmi_files/parameters.xml";
    else
        file_name = file_path;

    QString file_back = file_name + QString("_back");
    if(access(file_back.toUtf8().data(), F_OK) == 0){
        unlink(file_back.toUtf8().data());
    }

    if(QFile::copy(file_name, file_back) == false){
        TLOG_INFO("copy failed errno:%d\n", errno);
        return -1;
    }

    QFile file(file_name);
    if(!file.open(QFile::WriteOnly | QFile::Truncate)){
        TLOG_INFO("open failed:%d\n", errno);
        return -1;
    }

    QTextStream out_stream(&file);
    out_stream.setCodec("utf-8");
    this->dom_doc.save(out_stream, 4);
    file.flush();
    fsync(file.handle());
    file.close();

    if(!file.open(QFile::ReadOnly)){
        TLOG_INFO("open failed:%d\n", errno);
        return -1;
    }
    QByteArray data = file.readAll();
    bool contain_result = data.contains("accessManagemnet");
    file.close();
    if(contain_result == false){
        TLOG_INFO("flush failed:%d\n", errno);
        unlink(file_name.toUtf8().data());
        QFile::copy(file_back, file_name);
        return -1;
    }

    system("sync");
    return 0;
}

/*********************************************************
 * @file      : parameters.cpp
 * @brief     : 0 = update, 1 = no update
 * @author    : fxs
 * @date      : 2026-06-17
 * @version   : V1.0
 *********************************************************/

int Parameters::update(QString file_path, QString& category_name, int tab, QString& parameter_name, QString& label, QString& new_value){

    TLOG_WARN("%s %s %d %s %s %s", file_path.toUtf8().data(), category_name.toUtf8().data(), tab, parameter_name.toUtf8().data(), label.toUtf8().data(), new_value.toUtf8().data());
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }

    QDomNodeList tab_list = node.childNodes();
    QDomNode tab_node = tab_list.at(tab);

    QDomNode sub_node = tab_node.firstChild();
    while(sub_node.toElement().attribute("name") != parameter_name && !sub_node.isNull()){
        sub_node = sub_node.nextSibling();
    }
    QDomNode para = sub_node.firstChild();
    while(para.toElement().tagName() != label && !para.isNull()){
        para = para.nextSibling();
    }

    if(!para.isNull()){
        QDomNode old_node = para.firstChild();

        QString old_value = old_node.nodeValue();
        if (old_value == new_value)
        {
            qDebug() << "old_value: " << old_value << " new_value :" << new_value;
            return 1;
        }

        para.firstChild().setNodeValue(new_value);
        TLOG_INFO("%s %s %d", new_value.toLocal8Bit().data(), __FILE__, __LINE__);
        qDebug() << "new_value: " <<new_value.toLocal8Bit().data() << " file: " << __FILE__;
        QDomNode new_node = para.firstChild();
        sub_node.replaceChild(new_node, old_node);
    }

    return 0;
}

int Parameters::query_one_value_withtab(QString &category_name,int tab,QString &label_name,QString &result,int &switch_flag) const
{
    QString lable = "range_set_num";
    QString is_switch = query(category_name, tab, label_name,lable);
    lable = "real_value";
    QString the_value = query(category_name, tab, label_name,lable);
    if(is_switch == "")
    {
        result =  the_value;
        switch_flag = 0;
        return 1;
    }
    else
    {
        QStringList the_value_list = query_range_set(category_name,tab,label_name);
        result =  the_value_list[the_value.toInt()];
        switch_flag = 1;
        return 1;
    }
}

int Parameters::query_one_value_withname(QString &category_name,QString &tab_name,QString &label_name,QString &result,int &switch_flag) const
{
    QString lable = "range_set_num";
    QString is_switch = query(category_name, tab_name, label_name,lable);
    lable = "real_value";
    QString the_value = query(category_name, tab_name, label_name,lable);
    if(is_switch == "")
    {
        result =  the_value;
        switch_flag = 0;
        return 1;
    }
    else
    {
        QStringList the_value_list = query_range_set_index(category_name,tab_name,label_name);
        result =  the_value_list[the_value.toInt()];
        switch_flag = 1;
        return 1;
    }
}

/*********************************************************
 * @file      : parameters.cpp
 * @brief     :
 * @author    : fxs
 * @date      : 2026-06-02
 * @version   : V1.0
 *********************************************************/

QList<QStringList> Parameters::read(QString& category_name,QList <info_prara> &list_num, int tab_index, QList <QList <int>> &index_list,int &range_size) const{
    QList<QStringList> list;
    QStringList user_list;
    QString users;
    QString str;

    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点

    int para_type = 0; //0表示数字型， 1表示开关型

    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }

    QDomNodeList tab_list = node.childNodes();
    int sub_block = tab_index;
    range_size = tab_list.size();   //6ge

    //for(int sub_block=0; sub_block < tab_list.size(); sub_block++)
    QStringList para_list;
    QList <int> list_int;

    QDomNodeList node_list = tab_list.at(sub_block).childNodes();

    if(tab_list.at(sub_block).toElement().tagName() =="subblock")//找出只读项
    {
        QDomNode para_subblock = tab_list.at(sub_block);
        QString test = para_subblock.toElement().attribute("type");
        if(test == "readonly")
        {
            struct info_prara data;
            data.name = para_subblock.toElement().attribute("name");
            data.tab = sub_block;
            list_num.push_back(data);
        }
    }

    for(int i=0; i<node_list.size(); i++){
        QDomNode element = node_list.at(i);

        str.clear();
        QString disable_item = element.toElement().attribute("disableitem");
        user_list.clear();
        QString typeStr;
        bool is_ip = false;
        bool is_enum = false;

        QDomNodeList typeList = element.childNodes();
        for(int k = 0; k < typeList.size(); k++)
        {
            QString tag = typeList.at(k).toElement().tagName();
            if(tag == "type")
            {
                typeStr = typeList.at(k).toElement().text();
                qDebug() << "typeStr " << typeStr;
                if(typeStr == "ip") is_ip = true;
            }
            if(tag == "range_set_num")
            {
                is_enum = true;
            }
        }

        if(disable_item.size() > 0 && !is_ip)
        {
            QStringList items = disable_item.trimmed().split('|', QString::SkipEmptyParts);
            QString value_item = element.toElement().attribute("value");

            QString lable = "range_set_num";
            QString is_switch = query(items[0], items[1], items[2],lable);
            lable = "real_value";
            QString the_value = query(items[0], items[1], items[2],lable);
            if(is_switch == "")
            {
                if(the_value == value_item)
                {
                    continue;//非开关类型如果相等就不显示
                }
            }
            else
            {
                QStringList the_value_list = query_range_set_index(items[0],items[1],items[2]);
                if(value_item == the_value)
                {
                    continue;//开关型 如果相等就不显示
                }
            }
        }

        QDomNodeList label_list = element.childNodes();
        para_type = is_enum ? 1 : 0;; //重置为数字型

        for(int j=0; j<label_list.size(); j++){
            QString tag = label_list.at(j).toElement().tagName();
            if(tag == "range_set_num"){
                para_type = 1; //有集合的标签，表明为开关型
                break;
            }
        }
        QString index_real_value;
        for(int j=0; j<label_list.size(); j++){
            QDomNode para = label_list.at(j);
            if(para.toElement().tagName() == "descriptor"){
                str = para.toElement().text() + ":";
            }
            if(para.toElement().tagName() == "access"){

                users = para.toElement().text();
                //Q_ASSERT(users.size() > 0);
                //TLOG_INFO("i:%d sub_block:%d\n", i, sub_block);
                if(users.size() > 0)
                {
                    user_list = users.trimmed().split('|', QString::SkipEmptyParts);
                }
            }
            else {
                //TLOG_INFO("i:%d sub_block:%d\n", i, sub_block);
                //int k1 = 0;
            }

            if(is_ip)
            {
                if(para.toElement().tagName() == "real_value")
                {
                    str += para.toElement().text();
                }

                if(para.toElement().tagName() == "end")
                {
                    if(HasPrority(user_list))
                    {
                        para_list << str;
                        list_int.push_back(i);
                    }
                    str.clear();
                }

                continue;
            }

            if(para_type == 0){//数字型
                if(para.toElement().tagName() == "real_value"){
                    str += para.toElement().text();
                }
                if(para.toElement().tagName() == "unit"){ //要注意对于没有单位的参数，要特别处理，暂时未做
                    str += para.toElement().text();
                }
            }else if(para_type == 1){ //开关型
                if(para.toElement().tagName() == "real_value"){
                    index_real_value = para.toElement().text();
                }
                if((para.toElement().tagName() == "range_set") && (para.toElement().attribute("index") == index_real_value)){
                    PortModeInfo info = queryPortModeInfo(category_name, tab_index, element.toElement().attribute("name"));
                    str += info.modeText;
                    if(!info.baudrateList.isEmpty())
                    {
                        str += " ";
                        str += info.baudrateList[info.baudrateIndex];
                    }
                }
            }

            if(para.toElement().tagName() == "end"){
                if(HasPrority(user_list))
                {
                    para_list << str;
                    list_int.push_back(i);
                }
                else {
                    QString test = str;
                    //int k = 1;
                }
                str.clear();
                //users.clear();
            }
        }
    }
    list << para_list;
    index_list.push_back(list_int);

    qDebug() << "list: "<< list << "5689---";
    qDebug() << "index_list" << index_list << "88596+++";

    return list;
}

QList<QStringList> Parameters::read(QString& category_name,QList <info_prara> &list_num,QList <QList <int>> &index_list) const{
    QList<QStringList> list;
    QStringList user_list;
    QString users;
    QString str;
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    int para_type = 0; //0表示数字型， 1表示开关型
    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }
    QDomNodeList tab_list = node.childNodes();
    for(int sub_block=0; sub_block < tab_list.size(); sub_block++){
        QStringList para_list;
        QList <int> list_int;
        QDomNodeList node_list = tab_list.at(sub_block).childNodes();
        if(tab_list.at(sub_block).toElement().tagName() =="subblock")//找出只读项
        {
            QDomNode para_subblock = tab_list.at(sub_block);
            QString test = para_subblock.toElement().attribute("type");
            if(test == "readonly")
            {
                struct info_prara data;
                data.name = para_subblock.toElement().attribute("name");
                data.tab = sub_block;
                list_num.push_back(data);
            }
        }

        //int size = node_list.size();
        for(int i=0; i<node_list.size(); i++){
            QDomNode element = node_list.at(i);
            QString disable_item = element.toElement().attribute("disableitem");
            user_list.clear();
            if(disable_item.size() > 0)
            {
                QStringList items = disable_item.trimmed().split('|', QString::SkipEmptyParts);
                QString value_item = element.toElement().attribute("value");

                QString lable = "range_set_num";
                QString is_switch = query(items[0], items[1], items[2],lable);
                lable = "real_value";
                QString the_value = query(items[0], items[1], items[2],lable);
                if(is_switch == "")
                {
                    if(the_value == value_item)
                    {
                        continue;//非开关类型如果相等就不显示
                    }
                }
                else
                {
                    QStringList the_value_list = query_range_set_index(items[0],items[1],items[2]);
                    if(value_item == the_value)
                    {
                        continue;//开关型 如果相等就不显示
                    }
                }
            }

            QDomNodeList label_list = element.childNodes();
            para_type = 0; //重置为数字型
            for(int j=0; j<label_list.size(); j++){
                if(label_list.at(j).toElement().tagName() == "range_set_num"){
                    para_type = 1; //有集合的标签，表明为开关型
                    break;
                }
            }
            QString index_real_value;
            for(int j=0; j<label_list.size(); j++){
                QDomNode para = label_list.at(j);
                if(para.toElement().tagName() == "descriptor"){
                    str += para.toElement().text() + ":";
                }
                if(para.toElement().tagName() == "access"){

                    users = para.toElement().text();
                    //TLOG_INFO("i:%d sub_block:%d\n", i, sub_block);
                    if(users.size() > 0)
                    {
                        user_list = users.trimmed().split('|', QString::SkipEmptyParts);
                    }
                }
                else {
                    //TLOG_INFO("i:%d sub_block:%d\n", i, sub_block);
                    //int k1 = 0;
                }
                if(para_type == 0){//数字型
                    if(para.toElement().tagName() == "real_value"){
                        str += para.toElement().text();
                    }
                    if(para.toElement().tagName() == "unit"){ //要注意对于没有单位的参数，要特别处理，暂时未做
                        str += para.toElement().text();
                    }
                }else if(para_type == 1){ //开关型
                    if(para.toElement().tagName() == "real_value"){
                        index_real_value = para.toElement().text();
                    }
                    if((para.toElement().tagName() == "range_set") && (para.toElement().attribute("index") == index_real_value)){
                        if (sub_block == 0) {
                            PortModeInfo info = queryPortModeInfo(category_name, 0, element.toElement().attribute("name"));
                            str += info.modeText;
                            if(!info.baudrateList.isEmpty())
                            {
                                str += " ";
                                str += info.baudrateList[0];
                            }
                        } else {
                            str += para.toElement().text();
                        }
                    }
                }
                if(para.toElement().tagName() == "end"){
                    if(HasPrority(user_list))
                    {
                        para_list << str;
                        list_int.push_back(i);
                    }
                    else {
                        QString test = str;
                        //int k = 1;
                    }
                    str.clear();
                    //users.clear();
                }
            }
        }
        list << para_list;
        index_list.push_back(list_int);
    }
    return list;
}

QList<QStringList> Parameters::read(QString& category_name,QList <info_prara> &list_num) const{
    QList<QStringList> list;

    QStringList user_list;
    QString users;

    QString str;
    std::lock_guard<std::mutex> lck(parameter_mutex);
    QDomElement root = this->dom_doc.documentElement(); //返回根节点
    QDomNode node = root.firstChild(); //获得第一个子节点
    int para_type = 0; //0表示数字型， 1表示开关型
    while(node.toElement().attribute("category") != category_name && !node.isNull()){
        node = node.nextSibling();
    }
    QDomNodeList tab_list = node.childNodes();
    for(int sub_block=0; sub_block < tab_list.size(); sub_block++){
        QStringList para_list;
        QDomNodeList node_list = tab_list.at(sub_block).childNodes();
        if(tab_list.at(sub_block).toElement().tagName() =="subblock")//找出只读项
        {
            //tab_list.at(sub_block).toElement().attributes("name");
            QDomNode para_subblock = tab_list.at(sub_block);
            QString test = para_subblock.toElement().attribute("type");
            if(test == "readonly")
            {
                //int k = 0;
                struct info_prara data;
                data.name = para_subblock.toElement().attribute("name");
                data.tab = sub_block;
                list_num.push_back(data);
            }
            //para_subblock.toElement().attributes("name");
        }
        for(int i=0; i<node_list.size(); i++){
            QDomNode element = node_list.at(i);
            QDomNodeList label_list = element.childNodes();
            para_type = 0; //重置为数字型
            //判断是数字型还是开关型参数
            for(int j=0; j<label_list.size(); j++){
                if(label_list.at(j).toElement().tagName() == "range_set_num"){
                    para_type = 1; //有集合的标签，表明为开关型
                    break;
                }
            }
            QString index_real_value;
            for(int j=0; j<label_list.size(); j++){
                QDomNode para = label_list.at(j);
                if(para.toElement().tagName() == "descriptor"){
                    str += para.toElement().text() + ":";
                }
                if(para.toElement().tagName() == "access"){

                    users = para.toElement().text();
                    Q_ASSERT(users.size() > 0);
                    if(users.size() > 0)
                    {
                        user_list = users.trimmed().split('|', QString::SkipEmptyParts);
                    }

                }
                if(para_type == 0){//数字型
                    if(para.toElement().tagName() == "real_value"){
                        str += para.toElement().text();
                    }
                    if(para.toElement().tagName() == "unit"){ //要注意对于没有单位的参数，要特别处理，暂时未做
                        str += para.toElement().text();
                    }
                }else if(para_type == 1){ //开关型
                    if(para.toElement().tagName() == "real_value"){
                        index_real_value = para.toElement().text();
                    }
                    if((para.toElement().tagName() == "range_set") && (para.toElement().attribute("index") == index_real_value)){
                        str += para.toElement().text();
                    }
                }
                if(para.toElement().tagName() == "end"){
                    //para_list << str;
                    //str.clear();

                    if(HasPrority(user_list))//有权限就显示
                    {
                        para_list << str;
                        //str.clear();
                    }
                    str.clear();
                }
            }
        }
        list << para_list;
    }
    return list;
}




QStringList Parameters::read(QString& category_name,int tab,QList <info_prara> &list_num,QList <QList <int>> &index_list) const{
    return read(category_name,list_num,index_list).at(tab);
}

int Parameters::Encrypt(QString &src, QString &content)
{
    QString temp = src;

    for(int i = 0;i < 2; i++)
    {
        //QString temp = src;
        QByteArray text = temp.toLocal8Bit();
        QByteArray by = text.toBase64();
        temp = QString(by);
        //temp = content;
    }
    content = temp;
    return 0;
}

int Parameters::Decrypt(QString &src, QString &content)
{
    QString temp = src;

    for(int i = 0; i < 2; i++)
    {
        QByteArray text = temp.toLocal8Bit();
        QByteArray by = text.fromBase64(text);
        temp = QString::fromLocal8Bit(by);
    }
    content = temp;
    return 0;
}

//恢复xml文件中的默认值，s将default_value, 赋值给real_value
void Parameters::default_xml(){
}

//向广播地址192.168.*.255传送xml文件
void Parameters::transport()const{
}
