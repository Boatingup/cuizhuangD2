#ifndef ADDRESSSET_H
#define ADDRESSSET_H

//#include "mytypes.h"
#include <QString>
#include <QVector>
#include "../Resources/dbg_print.h"
typedef unsigned char uint8_t;

class AddressSet
{
private:
    AddressSet();

public:
    enum{
        ADDR_DIR_LEFT_TO_RIGHT = 0,
        ADDR_DIR_RIGHT_TO_LEFT = 1
    };

    enum{
        ACTION_EXE_CONCURRENT = 0,
        ACTION_EXE_DIR_LEFT_TO_RIGHT = 1,
        ACTION_EXE_DIR_RIGHT_TO_LEFT = 2,
        ACTION_EXE_DIR_GROUP = 3
     };

    enum LockType{
        KEY_LOCK = 1,
        PIR_LOCK = 2
     };


public:
    static AddressSet* getInstance(){
        if(address_set == nullptr){
            address_set = new AddressSet();
        }
        return address_set;
    }

    inline void set_local_address(uint8_t local_address){
        this->local = local_address;
    }

    inline uint8_t get_local_address()const{
        return local;
    }

    inline uint8_t get_min_address() const {
        return min_addr;
    }

    inline uint8_t get_max_address() const {
        return max_addr;
    }

    inline void set_min_address(uint8_t addr) {
        min_addr = addr;
    }

    inline void set_max_address(uint8_t addr) {
        max_addr = addr;
    }

    inline void set_destination_address(uint8_t dest_addr){
        this->destination = dest_addr;
    }

    inline uint8_t get_main_version()const{
        return main_version;
    }

    inline uint8_t get_child_version()const{
        return child_version;
    }

    inline void set_destination_address(uint8_t dest_addr_start, uint8_t dest_addr_end){
        this->destination_start = dest_addr_start;
        this->destination_end = dest_addr_end;
    }

    inline uint8_t get_destination_address()const {
        return this->destination;
    }

    inline uint8_t get_destination_address_start()const{
        return this->destination_start;
    }

    inline uint8_t get_destination_address_end()const{
        return this->destination_end;
    }

    inline void set_stop_last(uint8_t stop_addr){
        this->address_set->stop_last = stop_addr;
    }

    inline void set_lock_last(uint8_t lock_addr){
        this->address_set->lock_last = lock_addr;
    }

    inline void set_lock_type(uint8_t lock_type){
        this->address_set->lock_type = lock_type;
    }

    inline uint8_t get_lock_type()const{
        return this->address_set->lock_type;
    }

    inline uint8_t get_stop_last()const{
        return this->address_set->stop_last;
    }

    inline uint8_t get_lock_last() const{
        return this->address_set->lock_last;
    }

    inline int get_address_direction()const{
        return this->address_direction;
    }

    inline void set_address_direction(int dir){
        this->address_direction = dir;
    }

    inline int get_action_exe_direction() const {
        return this->action_exe_direction;
    }

    inline void set_action_exe_direction(int dir){
        this->action_exe_direction = dir;
    }

    inline void add_shearer_position(uint8_t support_id){//增加煤机附近支架架号
        this->shearer_position_sum += support_id;
        this->shearer_position_num++;
    }

    inline void sub_shearer_position(uint8_t support_id){//减少煤机附近支架架号
        this->shearer_position_sum -= support_id;
        this->shearer_position_num--;
    }

    inline int get_shearer_position() {
        if(this->shearer_position_num == 0)
        {
            return 0;
        }
        return (this->shearer_position_sum/this->shearer_position_num);
    }


  //  inline void set_shearer
private:
    static AddressSet* address_set;

private:
    //addressing direction
    int address_direction;
    int action_exe_direction;

public:
    QString action_id_str; //added by xufeng 2022.2.9

public:

    QString follow_step; //added by xufeng 2022.3.22

    uint8_t action_id;  //added by xufeng 2022.1.14

    uint8_t follow_dir;     //added by xufeng 2022.3.17
private:
    /* Controller address for CAN node */
    uint8_t local;
    uint8_t destination;

    uint8_t main_version;
    uint8_t child_version;

    /* destination single Controller address for CAN node. */
    uint8_t destination_start;
    uint8_t destination_end;

    /* destination single Controller address for CAN node. */
    uint8_t cancel_start;
    uint8_t cancel_end;

    /* STOP or LOCK controller address for CAN node */
    uint8_t stop_last; /* 最后一个发来急停消息的架号, and will display the last one */
    uint8_t lock_last; /* 最后一个发来闭锁消息的架号 */

    bool is_local_stop;/* 是否由本架引起的急停，在急停状态时设置和查看 */
    bool is_local_lock;/* 是否由本架引起的闭锁，在闭锁状态时设置和查看 */
    uint8_t lock_type;//如果是本架引起的闭锁，确认是闭锁类型 1 闭锁按键 2 检测到人体

    uint8_t min_addr;
    uint8_t max_addr;

    uint8_t  shearer_position_sum;//将所有接收到红外的支架架号求和，如果煤机离开某一支架，则减去对应架号
    uint8_t  shearer_position_num;//当前接收到红外信号的支架数量，随着煤机的移动增减




public:
    /* 任何发送来的急停或相关联的闭锁信号，或者自身急停闭锁时，都将其架号投入此vector中 */
    QVector<uint8_t> *vector_stop;
    QVector<uint8_t> *vector_lock;

};

#endif // ADDRESSSET_H
