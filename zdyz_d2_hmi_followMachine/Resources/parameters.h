#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QtXml>
#include <QStandardItemModel>
#include "guiconfig.h"
//add for aes testing  ---start
#include <iostream>
#include <stdio.h>
#include "../Resources/AES.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
//#include "./Resources/dbg_print.h"

#define HMI_FILES_PATH_BASE "/userdata/config/hmi_files"

struct info_prara{
    int tab;
    QString name;
    //QStandardItemModel *model;
};

struct PortModeInfo
{
    QString modeText;
    int baudrateIndex;
    int modeIndex;
    QStringList baudrateList;
};

struct Template_sheet{
    int id;//动作ID
    QString time = 0;//动作时间
    QString name;//动作名
};

struct sensor_info{
    int index;
    QString name;
    QString value;
    QString dip_value[3];
    float max;
    float min;
    //QStandardItemModel *model;
};

typedef struct{
    int angle_num;
    float angle1_x;
    float angle1_y;
    float angle2_x;
    float angle2_y;
    float angle3_x;
    float angle3_y;
    float angle4_x;
    float angle4_y;
    float angle5_x;
    float angle5_y;
    int L1;
    int L2;
    int L3;
    int L4;
    int L5;
    int L6;
    int L11;
    int L12;
    int L13;
    int L14;
}CALIBRATION_PARAM_S;

class Parameters
{
private:
    Parameters();
public:
    static Parameters* getInstance(){
        if(parameters == nullptr){
            parameters = new Parameters();
        }
        return parameters;
    }

public:
    //void get_calbration_param();
    bool get_support_type(QString & type_name, int & type_index);
    int load_parameter_file_local(QString& file_path);
    int getBlockNumber(QString& category_name);
    QStringList query_range_set_index(QString& category_name, QString& tab_name, QString& parameter_name) const;
    PortModeInfo queryPortModeInfoByIndex(QString category, int tab, QString parameter_name, int modeIndex) const;
    PortModeInfo queryPortModeInfo(QString category, int tab, QString parameter_name) const;
    int load_parameter_file(QString& file_path); //return 0 if sucessed, -1 for opening file failure
    int queryTemplate(QString & intervals_string,QString & id_list_string,QStringList &intervals_list,QStringList &id_list);
    int WriteTemplate(QList <Template_sheet> & list_template);
    int query_for_menuconfig(QString& category_name,QList <mainPage_uigonfig> & menuconfig);

    bool HasPrority(QStringList &list) const;

    QDomNode query_subnode(QString& category_name, QString& tab_name, QString& parameter_name) const;
    QString query(QString& category_name, QString& tab_name, QString& parameter_name, QString& label) const;
    QString query(QString& category_name, int tab, QString& parameter_name, QString& label) const;
    QString query_port_mode_by_id(QString& category_name, QString& tab_name, QString& parameter_name, QString &label, QString& index) const;
    int query_one_value_withname(QString &category_name,QString &tab_name,QString &label_name,QString &result,int &switch_flag) const;
    int query_one_value_withtab(QString &category_name,int tab,QString &label_name,QString &result,int &switch_flag) const;
    QString query_parameter_name(QString& category_name, QString& tab_name, int index) const;
    QString query_parameter_name(QString& category_name, int tab, int index) const;
    QString query_subtomap(QString& category_name,  QString& parameter_name, QHash <QString,int> &map_data, QList <sensor_info> &list_data) const;
    QString query_subtomap_local(QString& category_name,  QString& parameter_name, QHash <QString,int> &map_data, QList <sensor_info> &list_data) const;

    QString query_tab_name_by_index(QString& category_name, int tab) const;
    QString query_element_name(QString& category_name, QString& tab_name, QString& parameter_name, int index) const;
    int query_element_index(QString& category_name, QString& tab_name, QString& parameter_name, QString& name) const;
    QStringList query_range_set(QString& category_name, QString& tab_name, QString& parameter_name) const;
    QStringList query_range_set(QString& category_name, int tab, QString& parameter_name) const;
    QStringList query_range_set(QString& category_name, int tab, QString& parameter_name,QStringList &name_list) const;
    QStringList query_error_set(QString& category_name, QString& tab_name, QString& parameter_name,QMap <QString,int> & error_map) const;//获取系统所有错误

    int update(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_vaule); //return 0 if sucessed
    int update(QString file_path, QString& category_name, int tab, QString& parameter_name, QString& label, QString& new_vaule); //return 0 if sucessed
    int update_range_set(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& range_set_new_name);
    int updateAndSave(QString file_path, QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_value);


    int update_cache(QString& category_name, QString& tab_name, QString& parameter_name, QString& label, QString& new_value);//写入缓存
    int update_sync(QString file_path);//写入磁盘

    //为提高加载页面效率，本函数一次只加载一个查询一个tab 页
    QList<QStringList> read(QString& category_name,QList <info_prara> &list_num, int tab_index, QList <QList <int>> &index_list,int &range_size) const;

    QList<QStringList> read(QString& category_name) const;
	QList<QStringList> read(QString& category_name,QList <info_prara> &list_num) const;
    QList<QStringList> read(QString& category_name,QList <info_prara> &list_num,QList <QList <int>> &index_list) const;
    QStringList read(QString& category_name,int tab,QList <info_prara> &list_num,QList <QList <int>> &index_list) const;
    int getAccess(QString& category_name,QString& tab_name,QString& parameter_name,QString& label ,QStringList &usr_list);

    QStringList read(QString& category_name, int tab) const;
    int getAccess(QString& category_name,int tab,int index,QString &label_name);

    QHash<QString, int> sensor_map;
    QHash<QString, int> ad_map;

    void default_xml(); //恢复xml文件中的默认值，s将default_value, 赋值给real_value
    void transport()const;  // 通过网络向其他架传送parameters.xml file

    int Encrypt(QString &src, QString &content);
    int Decrypt(QString &src, QString &content);

private:
    QDomDocument dom_doc;
private:
    static Parameters* parameters;

public:
    QMap<QString, QString> category_name;
    QMap<QString, int> category_index;
    QList<QStringList> tab_name;
    //CALIBRATION_PARAM_S high_calibration;
    int control_status = 0;
};

#endif // PARAMETERS_H
