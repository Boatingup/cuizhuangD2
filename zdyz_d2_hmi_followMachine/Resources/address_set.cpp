#include "address_set.h"
#include <QString>
#include <QVector>
#include <QFile>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "../window/mainpage.h"

AddressSet* AddressSet::address_set = nullptr;


AddressSet::AddressSet()
{
    this->local = 0; //目前本架架号固定为10号，后期要从文件中读取
    this->destination = 0;

    this->main_version = 1;
    this->child_version = 0;

    this->address_direction = AddressSet::ADDR_DIR_LEFT_TO_RIGHT;
    this->action_exe_direction = AddressSet::ACTION_EXE_DIR_LEFT_TO_RIGHT;

    this->destination_start = 0;
    this->destination_end = 0;

    this->cancel_start = 0;
    this->cancel_end = 0;

    this->stop_last = 0;
    this->lock_last = 0;
    this->lock_type = 0;

    this->vector_stop = new QVector<uint8_t>(10);
    this->vector_lock = new QVector<uint8_t>(10);

    this->min_addr = 1;
    this->max_addr = 0xEF; //239

    this->shearer_position_sum = 0;
    this->shearer_position_num = 0;

    this->follow_dir = 0;

}
