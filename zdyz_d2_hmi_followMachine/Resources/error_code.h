/**
 * @file   my_errorcode.h
 * @author
 * @date   2018.9.1
 *
 * @brief
 *         通用错误� �定义及实现
 *
 * @note
 *         1、错误� �从0开始计算，顺序添�� 。
 *         2、实际使用时，除E_OK外，错误� �可以是负数(需要手工添�� '-')，也可以是正数。
 *         3、错误� �可以是任何数字，但不在枚举之列则显示未知错误� �。
 *
 * @log
 *
 */
#ifndef _MY_ERRORCODE_H
#define _MY_ERRORCODE_H
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
enum errorcode{
    E_OK = 0,         		///< 成功
    E_WRITE_FAIL,           	///< 写服务器OPC数据错误
    E_READ_FAIL,		///读服务器OPC数据错误
    E_READ_CONFIG_ERROR,       ///读取配置文件错误
    E_CONNECT_SERVER,          /// 连接服务器错误
    E_GET_LOCAL_ADDRESS,       ///  获取IP地址错误
    E_COMMAND_ERROR,           ///  数据异常
    E_BROWSE_SERVER_ERROR,     ///  获取服务器数据错误
    E_RECEIVE_MSGQUEUE_ERROR,     ///  接收消息队列数据错误
    E_SEND_MSGQUEUE_ERROR,///  发送消息队列数据错误

    E_END,                     ///< �� 位，�� 实际作用
};

const char* get_errorcode(int ec);
#endif
