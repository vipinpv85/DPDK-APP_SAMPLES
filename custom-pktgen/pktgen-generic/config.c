#include <string.h>
#include <stdlib.h>

#include "config.h"

uint8_t configCount = 0;
port_config_t config [PKTGEN_MAXPORTS] = {{0}};

/* user defined sizes */
//const uint16_t cSize[C_MAXCOUNT] = {64, 64, 128, 134};
//const uint16_t gtpUSize[U_MAXCOUNT] = {134, 134};
const uint16_t arpSize[ARP_MAXCOUNT] = {64, 95}; /*ARP*/
const uint16_t icmp4Size[ARP_MAXCOUNT] = {64, 95}; 
const uint16_t icmp6Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t ipv4Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t ipv6Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t tcp4Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t tcp6Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t udp4Size[ARP_MAXCOUNT] = {64, 95};
const uint16_t udp6Size[ARP_MAXCOUNT] = {64, 95};

/* user defined pkts */
//const uint8_t c[C_MAXCOUNT][800] = {C_ECHOREQ, C_ECHOREP, C_CREATEPDU_REQ, C_CREATEPDU_REP};
//const uint8_t gtpU[U_MAXCOUNT][1500] = {U_ICMPREQ, U_ICMPREP};
const uint8_t arp[ARP_MAXCOUNT][255] = {ARP_REQUEST, ARP_RESPONSE};
const uint8_t icmp4[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t icmp6[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t ipv4[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t ipv6[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t tcp4[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t tcp6[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t udp4[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};
const uint8_t udp6[ARP_MAXCOUNT][255]={ARP_REQUEST, ARP_RESPONSE};

static inline int getInt(const char *string)
{
    int i, stringLength = strlen(string);

    for(i = 0; i < stringLength; i++)
    {
        if((isdigit(string [i]) == 0))
            return -1;

    }

    return (atoi(string));
}

int32_t loadConfig(void)
{
    struct rte_cfgfile *file = NULL;
    char **section_names = NULL;
    char *ptr;
    struct rte_cfgfile_entry entries[32];
    int32_t sectCount = 0, entrCount = 0, i = 0, j = 0, ret = -1;

    file = rte_cfgfile_load(CONFIGFILE, 0);
    if (file == NULL) {
        printf("Cannot load configuration profile %s\n", CONFIGFILE);
        return -1;
    }

    sectCount = rte_cfgfile_num_sections(file, PKTGEN_INTFTAG, strlen(PKTGEN_INTFTAG));
    //printf("\n section starting with INTF are %d", sectCount);
    //sectCount = rte_cfgfile_num_sections(file, NULL, 0);

    if (sectCount <= PKTGEN_MAXPORTS) {
        section_names = malloc(sectCount * sizeof(char *));

        for (i = 0; i < sectCount; i++)
            section_names[i] = malloc(10);

        rte_cfgfile_sections(file, section_names, sectCount);
        for (i = 0; i < sectCount; i++) {
            //printf("\n\n section - %s", section_names[i]);

            entrCount = rte_cfgfile_section_num_entries(file, section_names[i]);
            //printf("\n - entries Count: %d", entrCount);
            //printf("\n - ENTRY : VALUE ");
            //printf("\n ----------------");

            if (entrCount < 4) {
                /* MAX sections are 3 per interface */
                ret = rte_cfgfile_section_entries(file, section_names[i], entries, entrCount);

                config[i].prtIndex = getInt(section_names[i] + strlen(PKTGEN_INTFTAG));

                //printf("\n Before: Port: %d gtp type %d", i, config[i].type);
                for (j = 0; j < ret; j++) {
                    //printf("\n %15s : %-15s", 
                    //         entries[j].name, entries[j].value);

                    switch (strlen(entries[j].name)) {

                        case (4):
                            if (STRCMP("type", entries[j].name) == 0)
                                //printf("\n %s  --- %d", entries[j].name, config[i].type);
                                config[i].type = 
                                    (STRCMP("ARP",entries[j].value) == 0)?ARP:
                                    (STRCMP("ICMP4",entries[j].value) == 0)?ICMP4:
                                    (STRCMP("ICMP6",entries[j].value) == 0)?ICMP6:
                                    (STRCMP("IPV4",entries[j].value) == 0)?IPV4:
                                    (STRCMP("IPV6",entries[j].value) == 0)?IPV6:
                                    (STRCMP("TCP4",entries[j].value) == 0)?TCP4:
                                    (STRCMP("TCP6",entries[j].value) == 0)?TCP6:
                                    (STRCMP("UDP4",entries[j].value) == 0)?UDP4:
                                    (STRCMP("UDP6",entries[j].value) == 0)?UDP6:
                                                        0xff;

                                //printf("\n ****** %s  --- %d", entries[j].name, config[i].type);
                            break;

                        case (5):
                            if (STRCMP("index", entries[j].name) == 0) {
                                config[i].pktIndex = strtol(entries[j].value, &ptr, 10);
                            }
                            break;

                        default:
                            printf("\n ERROR: unexpected entry %s with value %s", \
                                   entries[j].name, entries[j].value);
                            ret = rte_cfgfile_close(file);
                            return -2;
                    } /* update per entry */
                } /* iterate entries */
                //printf("\n After: Port: %d gtp type %d", i, config[i].type);
            } /* entry count */
        } /* per section */
        configCount = i;
    } /* section count*/

    ret = rte_cfgfile_close(file);
    return 0;
}



