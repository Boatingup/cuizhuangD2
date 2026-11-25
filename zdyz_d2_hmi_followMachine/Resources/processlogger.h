#ifndef PROCESSLOGGER_H
#define PROCESSLOGGER_H
#include <stdio.h>
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
class ProcessLogger
{
public:
    ProcessLogger();
    //int Process_Log(char loginfo, char * filename, int line,int level);
    int Process_Log(const char* loginfo, const char * filename, int line,int level);
private:

};

#endif // PROCESSLOGGER_H
