#ifndef __NODE__
#define __NODE__

#include "config.h"

/* DEFINES */
#define RTE_TEST_TX_DESC_DEFAULT (4096)
#define RTE_TEST_RX_DESC_DEFAULT (1024)

/* STRUCTURES */
typedef struct numaInfo_s {
  uint8_t port1G_map[4];
  uint8_t port10G_map[4];
  uint8_t port40G_map[4];

  uint8_t port1G_count[4];
  uint8_t port10G_count[4];
  uint8_t port40G_count[4];

  uint64_t core_map[4];
  uint8_t  core_count[4];
} numaInfo_t;

/* FUNCTION DECLARATION */
uint8_t getCount1gPorts(numaInfo_t *nodeCorePortInfo);
uint8_t getCount10gPorts(numaInfo_t *nodeCorePortInfo);
uint8_t getCount40gPorts(numaInfo_t *nodeCorePortInfo);

int32_t populateNodeInfo(numaInfo_t *nodeCorePortInfo);
void    displayNodeInfo(void);

#endif /* __NODE__ */
