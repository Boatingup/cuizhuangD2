#ifndef DRIVER_CHN_CONFIG_H
#define DRIVER_CHN_CONFIG_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QEvent>
#include <QString>

#include "widget_stack.h"
#include "./Resources/parameters.h"
#include "./threads/comm_acs_socket_thread.h"
#include "fault_diagnosis_diaplsy.h"
#include "./Resources/key_value.h"
//#include "driver_chn_config_child.h"
//#include "./threads/para_synchronize_file.h"


#define ACTION_NONE                             0x00000000U//通道全关闭
#define ACTION_J5_K1                            0x00000001U
#define ACTION_J5_K2                            0x00000002U
#define ACTION_J3_K3                            0x00000004U
#define ACTION_J3_K4                            0x00000008U
#define ACTION_J1_K5                            0x00000010U
#define ACTION_J1_K6                            0x00000020U
#define ACTION_J11_K7                           0x00000040U
#define ACTION_J11_K8                           0x00000080U
#define ACTION_J9_K9                            0x00000100U
#define ACTION_J9_K10                           0x00000200U
#define ACTION_J7_K11                           0x00000400U//J7第二路
#define ACTION_J7_K12                           0x00000800U//J7第一路
#define ACTION_J6_K13                           0x00001000U
#define ACTION_J6_K14                           0x00002000U
#define ACTION_J4_K15                           0x00004000U
#define ACTION_J4_K16                           0x00008000U
#define ACTION_J2_K17                           0x00010000U
#define ACTION_J2_K18                           0x00020000U
#define ACTION_J12_K19                          0x00040000U
#define ACTION_J12_K20                          0x00080000U
#define ACTION_J10_K21                          0x00100000U
#define ACTION_J10_K22                          0x00200000U
#define ACTION_J8_K23                           0x00400000U
#define ACTION_J8_K24                           0x00800000U
#define ACTION_J13_K25                          0x01000000U
#define ACTION_J13_K26                          0x02000000U
#define ACTION_J14_K27                          0x04000000U
#define ACTION_J14_K28                          0x08000000U

typedef struct _action_driver_key_info
{
    QString key_name;
    QString driver_chn;
}ACTION_DRIVER_KEY_INFO_S;

typedef struct _driver_chn
{
    QString driver1[4];
    QString driver2[4];
}DRIVER_CHN_S;

typedef struct _action_match
{
    int action_no;
    bool occurpied;
}ACTION_MATCH_S;

namespace Ui {
class driver_chn_config;
}

class driver_chn_config : public QWidget
{
    Q_OBJECT

public:
    static driver_chn_config *getInstance(QString& category){
        if(single == nullptr){
            single = new driver_chn_config(category);
        }
        return single;
    }

public:
    static driver_chn_config* single;

public:
    explicit driver_chn_config(QString& category, QWidget *parent = nullptr);
    ~driver_chn_config();

    void load_config();


public:
     struct tableViewArr{
        QTableView* tableView_left;
     };

public:
    void fault_list_update();
    bool load_xml(QString& file_path);
    void update_driver_chn_config_xml();
protected:
     bool event(QEvent *event);
private:
    QItemSelectionModel *m_ItemSelectionModel;
    QStandardItemModel *m_pModle;
    QTabWidget *tabWidget;
    QList<QStringList> para_list;
    QList<tableViewArr> table_view_arr_list;

    QMap<QString, QString> driver_match_map;
    QMap<QString, QString> action_match_map;
    QMap<QString, QString> key_match_map;

    Parameters *para;
private:
    QDomDocument driver_xml;
    int row;
    int index;
    int tab;
    int display_list_size;
    bool update_param_flag;
private:
    CommAcsSocketThread* comm_acs_thread;
    driver_param_remote_sync *driver_sync;
private:
    QString category;

    WidgetStack* widget_stack;

private slots:
    void save_slot(int flag);
    void update_slot();

private:
    Ui::driver_chn_config *ui;
};

#endif // DRIVER_CHN_CONFIG_H
