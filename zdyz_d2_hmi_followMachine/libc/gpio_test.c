#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "int_test.h"
#include "libintf.h"

static GPIO_ATTR_ST gpios[] = {
	{"IMX_GPIO4_23", IMX_GPIO4_23}, {"IMX_GPIO4_24",IMX_GPIO4_24},
	{"IMX_GPIO4_25", IMX_GPIO4_25}, {"IMX_GPIO4_26",IMX_GPIO4_26},
	{"IMX_GPIO4_27", IMX_GPIO4_27}, {"IMX_GPIO4_28",IMX_GPIO4_28},

};

static char *get_gpio_name(int gpio_index)
{
    int i;
    
    for (i = 0; i < ARRAY_SIZE(gpios); i++) 
    {
        if (gpio_index == gpios[i].gpio_index) 
        {
            return gpios[i].gpio_name;
        }
    }

    return NULL;
}

static int gpio_get_level(int fd, 	IOCTL_GPIO_ARG_ST *parg)
{
    int i;
    int ret = RET_SUCCESS;
    
    if (NULL == parg)
	{
        DEBUG_ERROR("Invalid argument!\n"); 
        return RET_FAILED;
	}

	if (NULL == get_gpio_name(parg->gpio_index))
	{
        DEBUG_ERROR("Invalid argument!\n"); 
        return RET_FAILED;
	}

    ret = ioctl(fd,IOCTL_GPIO_LEVEL_GET,parg);
    if (0 != ret)
    {
        ret = RET_FAILED;
    	DEBUG_ERROR("gpio '%s' 'IOCTL_GPIO_LEVEL_GET' error!errno = %d\n", 
    	            get_gpio_name(parg->gpio_index), ret);
    }
    else
    {
        if(parg->gpio_level >= 1)  // add by deng
					parg->gpio_level = 1;
        DEBUG_INFO("get gpio '%s' level '%d'\n", 
                 get_gpio_name(parg->gpio_index), parg->gpio_level);
    }

   return ret;
}

int gpio_test_connect(int fd)
{
    int i;
    int ret;
    int final_ret = RET_SUCCESS;
	  IOCTL_GPIO_ARG_ST arg;

    for (i = 0; i < ARRAY_SIZE(gpios); i++)
    {
        memset (&arg, 0, sizeof(arg));
        arg.gpio_index = gpios[i].gpio_index; 
        ret = gpio_get_level(fd, &arg);
        if ((RET_SUCCESS == ret))
        {
         //  DEBUG_INFO("get gpio '%s' level '%d'\n", 
        //               gpios[i].gpio_name, arg.gpio_level);
        }

        final_ret += ret;
    }
    return final_ret;
}