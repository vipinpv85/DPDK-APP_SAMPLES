# DPDK_PORTSTAT-1
Simple application to fetch stats for PCAP and TAP PMD interfaces for DPDK.
Application makes use of libpcap functionality to probe and matches against user ports.

Dependency
> DPDK non vdev ports it requires dpdk-procinfo application

Execution  
- Shared: ./build/port-stats --proc-type=secondary -d $RTE_SDK/$RTE_TARGET/lib/librte_pmd_virtio.so
- Static: ./build/port-stats --proc-type=secondary

>Note: in DPDK .config PCAP and TAP should be enabled
