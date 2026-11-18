#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QFile>

#include <cstring>

#include <unistd.h>
#include <fcntl.h>

#include "keyboardreadthread.h"
#include "../libc/libintf.h"


unsigned char previous_key_state[4] = {0x00, 0x00, 0x00, 0x00};
unsigned char current_key_state[4] = {0x00, 0x00, 0x00, 0x00};

unsigned char KeyboardReadThread::myxor(void *data, int length)
{
    unsigned char xorsum = 0;
    unsigned char *p = (unsigned char *)data;
    while(length--){
        xorsum ^= *p++;
    }
    return xorsum;
}


KeyboardReadThread::KeyboardReadThread(QObject *parent) :
    QThread(parent)
{

}

void KeyboardReadThread::setFousedWindow(QWidget *win){
    this->window = win;
}

void KeyboardReadThread::run()
{
    char readbuf[32];
    int nread = 0;
    while(true)
    {
        if(this->window == nullptr)
            continue;
        nread = ::get_uart_data(1, readbuf, 32);
        parse(readbuf, nread);
    }
}

int KeyboardReadThread::check_key_state(unsigned char* previous, unsigned char* current, int length)
{
    unsigned char result = 0;
    unsigned char key_value = 0;


    for(int byte = 0; byte < length; byte++)
    {
        //注意先传进来的是MSB
        result = *previous ^ *current ; //相异为真
        if(result != 0){ //==0 说明各位都相同，当前按键状态与前次没有变化
            for(int bit = 0; bit < 8; bit++)
            {
                if(result & 0x01)
                {
                    key_value = (3 - byte) * 8 + bit;
                    if((*current >> bit) & 0x01)
                    { //key pressed
                        //组织按键按下事件，发送给主线程
                        QCoreApplication::postEvent(this->window, new QKeyEvent(QEvent::User, key_value, Qt::NoModifier));
                        //测试时仅打印输出
                        //printf("Key %d pressed.\n", key_value);
                    }
                    else
                    { //key released
                        //组织按键抬起（释放）事件，发送给线程
                        QCoreApplication::postEvent(this->window, new QKeyEvent((QEvent::Type)(QEvent::User+1), key_value, Qt::NoModifier));
                        //测试时仅打印输出
                        //printf("Key %d released.\n", key_value);
                    }
                }
                result >>= 1;
            }
        }
        previous++;
        current++;
    }
    return 0;
}

int KeyboardReadThread::parse(char* data, int length)
{
    unsigned char *p = (unsigned char *)data;
    if(p[0] == 0xAA && p[1] == 0x55)
    {
        if(myxor(data, length - 1) != p[length - 1])
        {
            //printf("Xor check error.\n");
            return -1;
        }
//        for(int i = 0;i < length;i++)
//        {
//            printf("%x ",p[i]);
//        }
//        printf("\n");
        memcpy(previous_key_state, current_key_state, 4); //上次的当前键值状态是这一次的前次键值状态
        memcpy(current_key_state, &p[3], 4); //从第3字节开始的4个字节是当前键值状态
        //两次相异的位表示键值有变化，根据当前对应位的键值（0或1）组织成事件发送给窗口的主线程，目前仅输出测试
        check_key_state(previous_key_state, current_key_state, 4);
        return 0;
    }else{
        //printf("Frame headers error\n");
        return -2;
    }
}
