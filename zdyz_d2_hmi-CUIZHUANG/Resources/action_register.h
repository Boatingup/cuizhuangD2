#ifndef ACTION_REGISTER_H
#define ACTION_REGISTER_H


#include <QDomDocument>
#include <QtXml>

struct Action_info{
    QString action_id;
    QString action_name;
    bool is_additional;
    QString action_eng;
    int support_controled_type;
    QString gif_name;
    QString key_eng_name;
    bool is_start_timer = false;
};

class ActionRegister
{
private:
    ActionRegister();
public:
    static ActionRegister* getInstance(){
        if(action_register == nullptr){
            action_register = new ActionRegister();
        }
        return action_register;
    }

private:
    int load_action_xml_file(QString& file_path);
    int load_action_xml_file_local(QString& file_path);//no mutex
    void read(); //read key-value to action map
    void read_local(); //no mutex
public:
    QString find_descriptor_by_id(QString id) const;
    void reload();
    //int Find_Asy_action(QString name);
    int Find_Asy_action(QString name,QPair<QString, Action_info> &data);
    int update(QString file_path, QString& actionname,QString& label, QString& new_value);
    void Find_action(QStringList &id_list,QString &action_name);
    void find_gif_by_name(QString &key, QString &gif_info);
    int getActionByindex(int index,QString & action_key,QString & action_value);
    int getActionByindex(int index,QString & action_key,QString & action_value,QString &action_eng);
    void getGroupNeighbourActAditionalList();
    QString get_support_type();
public:
    QHash<QString, Action_info> action_map; //key:id, value:decriptor
    QHash<QString, Action_info> action_name_map; //key:name, value:info
    QHash<QString, Action_info> coal_map; // 放煤map数据结构
    QList <QPair <QString,Action_info>> action_list;//动作列表
    QList <QPair <QString,Action_info>> action_aditional_list;//附加动作列表
    QList <QPair <QString,Action_info>> group_action_aditional_list;//成组附加动作列表
    QList <QPair <QString,Action_info>> neighbour_action_aditional_list;//邻架附加动作列表
    QList <QPair <QString,Action_info>> interval_action_aditional_list;//隔架附加动作列表
    QList <QPair <QString,Action_info>> private_action_aditional_list;//本架附加动作列表


private:
    QDomDocument dom_doc;

private:
    static ActionRegister* action_register;
};




#if 0

#include <QString>

#include "mytypes.h"
#include "key_value.h"
/* 存在多个电磁阀驱动器时，对不同SBC的编号 */
#define SBC_NUM_1		0u
#define SBC_NUM_2		1u

class ActionRegister;

typedef union tagSBCNumChannel{
    uint32_t channelvalue;
    struct{
        uint32_t	channel : 24;
        uint32_t	number  : 8;
    }bits;
}SBCNumChannel_t;


typedef struct tagNode{
    struct tagNode*     next;
    QString             descriptor;
    int                 key;
    int                 reg_addr;
    int                 reg_bit;
    uint8_t             id;
    uint8_t             combinedActionNum;		/* 动作由几个通道进行复合，单动作时为1，最多由4个动作复合 */
    SBCNumChannel_t     sbcNumberAndChannel[4];	/* 复合动作的电磁阀驱动器号和相应的通道，单动作 */
}Node_t;


typedef enum{
    RESULT_OK = 0u,
    RESULT_FAIL,
    RESULT_NO_KEY,
    RESULT_BUFFER_FULL
}Result_t;

class ActionRegister
{
private:
    ActionRegister() ;

public:
    enum Action_ID{
        ACTION_ID_MOVE              = 1,//移架
        ACTION_ID_PUSH_RAIL         = 2,//推溜
        ACTION_ID_LIFT              = 3,//升柱
        ACTION_ID_DOWN              = 4,//降柱
        ACTION_ID_STRETCH_BALANCE   = 5,//伸平衡
        ACTION_ID_TAKE_BACK_BALANCE = 6,//收平衡
        ACTION_ID_LIFT_BOTTOM,//抬底
        ACTION_ID_SPRAY,//喷雾
        ACTION_ID_STRETCH_SIDE_GURAD,//伸侧护
        ACTION_ID_CONTRACT_SIDE_GURAD,//收侧护
        ACTION_ID_STRETCH_GURAD,//伸护帮
        ACTION_ID_CONTRACT_GURAD,//收护帮
        ACTION_ID_STRETCH_TAIL_BEAM,//伸尾梁
        ACTION_ID_CONTRACT_TAIL_BEAM,//收尾梁
        ACTION_ID_STRETCH_PLUGBOARD,//伸插板
        ACTION_ID_CONTRACT_PLUGBOARD,//收插板

        ACTION_ID_GROUP_AUTO_ACTING = 100,//成组自动动作执行中
        ACTION_ID_GROUP_ACT_PRE = 101,

        ACTION_ID_REMOTE_UPDATE = 200,

        ACTION_ID_DUMMY             =255

    };

    enum Key_zones{//按键功能分区
        FUNC_ZONE = 0,//功能区
        ACT_ZONE,    //动作区
        EXT_ZONE    //扩展区
    };

    enum Select_mode{   //选架模式
        SELECT_MODE_SINGLE = 0x11,
        SELECT_MODE_GROUP = 0x22
    };

    enum Channel{
        CH_1    = 1,
        CH_2    = 2,
        CH_3    = 3,
        CH_4    = 4,
        CH_5    = 5,
        CH_6    = 6
    };

    static ActionRegister* getInstance();

    Result_t insert(QString desc, int key,
                    uint8_t combinedActionNum, uint8_t sbcNum[],
                    uint32_t channel[], uint8_t action_ID, int reg_addr, int reg_bit);

    /* 快捷按键动作或辅助菜单中的动作，应通过Key值或ID值来查找相对应的Node，从中获得对应的电磁阀号，通道号,中文标识符等 */
    Result_t find_node_by_ID            (Node_t** node,  uint8_t id);
    Result_t find_node_by_key           (int key,       Node_t* node);
    Result_t find_channel_name_by_ID    (uint8_t id,    QString* channelname);
    Result_t find_channel_by_ID         (uint8_t id,    uint32_t* channel);
    Result_t find_ID_by_key             (int key,       uint8_t* id);
    Result_t find_offset_bit_by_ID      (uint8_t id, int *mb_offset, int *mb_bit);
    Result_t find_ID_by_offset              (int offset, int bit, uint8_t *id);


private:
    static ActionRegister* action_register;

    //QVector<Node_t*> nodes;
    Node_t nodes[48]; //max capcity is 48 for now
    int free_index;   //insert new node at free_index position

};

#endif



#endif // ACTION_REGISTER_H
