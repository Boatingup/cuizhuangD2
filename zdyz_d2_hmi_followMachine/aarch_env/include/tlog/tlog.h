
#ifndef __TLOG_H__
#define __TLOG_H__

#include <zlog.h>
#include <string>

extern int TLOG_INIT(const char *pcTag);
/*
	log
*/
#define TLOG_FATAL  dzlog_fatal
#define TLOG_ERROR  dzlog_error
#define TLOG_WARN   dzlog_warn
#define TLOG_NOTICE dzlog_notice
//#define TLOG_INFO   dzlog_info

#if 0
#define TLOG_INFO(str, args...)   printf("\033[0;31m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#else
#define TLOG_INFO   dzlog_info
#endif

#endif

