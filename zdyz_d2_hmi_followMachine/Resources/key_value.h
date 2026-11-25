#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include <QEvent>


class KeyValue{
public:
    enum Value{

        //崔庄订单按键
        KEY_NUM_0   = 4,
        KEY_NUM_1   = 0,
        KEY_F1      = 24,
        KEY_F2      = 25,
        KEY_F3      = 26,
        KEY_NUM_2   = 13,
        KEY_NUM_3   = 14,
        KEY_F4      = 27,

        KEY_F5      = 28,
        KEY_F6      = 29,
        KEY_NUM_4   = 15,
        KEY_NUM_5   = 3,
        KEY_ARROW_PLUS  = 20,
        KEY_ARROW_LEFT  = 22,
        KEY_ARROW_RIGHT = 23,
        KEY_NUM_6       = 1,

        KEY_NUM_7       = 10,
        KEY_ARROW_MINUS   = 21,
        KEY_ARROW_UP       = 17,
        KEY_ARROW_DOWN  = 16,
        KEY_PUSH_PULL   = 9,
        KEY_NUM_9       = 8,
        //KEY_FN          = 20,
        KEY_OK          = 19,
        KEY_CANCEL      = 18,

        KEY_AUX_L1 = 7,
        KEY_AUX_L2 = 6,
        KEY_AUX_L3 = 5,
        KEY_AUX_R1 = 4,
        KEY_AUX_R2 = 3,
        KEY_AUX_R3 = 2,

        KEY_F7       = 30,

    };

    enum{
        KEY_PRESSED  = QEvent::User,
        KEY_RELEASED = QEvent::User+1
    };
};


#endif // KEY_VALUE_H
