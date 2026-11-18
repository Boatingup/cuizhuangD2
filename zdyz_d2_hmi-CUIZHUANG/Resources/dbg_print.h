#ifndef DBG_PRINT_H
#define DBG_PRINT_H

#ifdef OPEN_ALL_DEBUG
#define DBG_INFO_OLD(str, args...)     printf("\033[1;32m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#else
#define DBG_INFO_OLD(str, args...)
#endif

#ifdef OPEN_DEBUG
#define DBG_INFO(str, args...)     printf("\033[1;32m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#define DBG_ERR(str, args...)      printf("\033[0;31m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#define DBG_WARN(str, args...)     printf("\033[1;33m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#else
#define DBG_INFO(str, args...)
#define DBG_ERR(str, args...)   printf("\033[0;31m %s>%s:#%d, \033[0m " str, __FILE__, __FUNCTION__, __LINE__, ## args)
#define DBG_WARN(str, args...)
#endif
#endif // DBG_PRINT_H
