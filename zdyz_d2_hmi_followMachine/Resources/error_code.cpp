#include "error_code.h"
#include <stdio.h>


static const char* faults[] = {
    [E_OK] = "Success",
    [E_WRITE_FAIL] = "App WriteDataError",
    [E_READ_FAIL] = "App ReadDataError",
    [E_READ_CONFIG_ERROR] = "App Read Config error",
    [E_CONNECT_SERVER] = "App Connect error",
    [E_GET_LOCAL_ADDRESS] = "App Sensor type not matched",
    [E_COMMAND_ERROR] = "APP data cmd unexpected",
    [E_BROWSE_SERVER_ERROR] = "App Cannot get server opc data", 
    [E_RECEIVE_MSGQUEUE_ERROR] = "App Get msg from MSG queue error ", 
    [E_SEND_MSGQUEUE_ERROR] = "App Send msg to MSG queue error ", 

};

/*
const char* get_errorcode(int ec)
{
    if (ec < 0) ec = -ec;
    printf("ec: %d\n", ec);
    if (ec < E_OK || ec >= E_END) return "Unknown error code";
    return faults[ec];
}
*/

const char* get_errorcode(int ec)
{
    //if (ec < 0) ec = -ec;
    //printf("ec: %d\n", ec);
    if (ec < E_OK || ec >= E_END) return "Unknown error code";
    else
    return faults[ec];
}



