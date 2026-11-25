#include "processlogger.h"

ProcessLogger::ProcessLogger()
{

}

int ProcessLogger::Process_Log(const char* loginfo, const char * filename, int line,int level)
{
    char logcontent[1024] ={0};
    sprintf(logcontent,"%s %s %d level:%d\n",loginfo,filename,line,level);
    printf("logcontent :%s\n",logcontent);
    if(level == 0)
    {
        TLOG_INFO("%s linenumber: %d filename: %s", loginfo, line, filename);
    }
    if(level == 1)
    {
        TLOG_WARN("%s linenumber: %d filename: %s", loginfo, line, filename);
    }
    if(level == 2)
    {
        TLOG_ERROR("%s linenumber: %d filename: %s", loginfo, line, filename);
    }
    return 0;
}
