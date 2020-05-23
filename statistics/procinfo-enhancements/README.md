# DPDK-DEBUG-ENHANCE_PROCINFO

## Motivation
DPDK procinfo tool is limited to debug for NIC stats|xstats and some basic memory information.
But fails to capture information like configuration or setup issues for NIC, Crypto, Eventedev,
Mempool, RING and other PMD.

## Scope
 - enhance for debuggging issue.
 - fetch stats and configuration of PMD
 - have tiemstamp counters for comparision
 - debug specific ring or mempool for custom meta data

## Upates
DPDK patchwork accepted to mainline hence will be availble from DPDK 19.02 onwards
