//config.h

/*I N C L U D E S */
#include<stdint.h>

/* D E F I N E S */
#define SEP "."


/* P R O T O T Y P E S */
int validate_ip(uint32_t ipaddress);
int validate_port(uint32_t *port);
