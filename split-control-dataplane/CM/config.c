// config.c

/* I N C L U D E S */
#include <stdio.h>
#include "config.h"

/* Function to validate ip address */
int validate_ip(uint32_t ipaddress)
{

    if ((ipaddress) && ((((ipaddress) & 0xff000000) == 0x00) || (((ipaddress) & 0x000000ff) == 0x00)))
    {
        return 0;
    }
    return 1;
}


/*Function to validate port address */
int validate_port(uint32_t *port)
{
    if ((*port >= 0) && (*port < 65536))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
