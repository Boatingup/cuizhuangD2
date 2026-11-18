#ifndef LOG_HEAD_H
#define LOG_HEAD_H

#include <QDebug>

#define LOG_DBG(msg) \
    qDebug() << "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "] " << msg

#endif
