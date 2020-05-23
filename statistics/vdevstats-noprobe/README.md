# DPDK-PRI-STATS

A secondary process application to pick up stats from primary DPDK & PCAP (vdev) interfaces without PCI probe or corrupting memory space.

To Run:
Build DPDK TARGET with PCAP env

Usuage
1) ./port-stats <PID>
  PID - optional - primary for vdev pcap interface
