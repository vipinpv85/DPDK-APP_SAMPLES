# DPDK_PRI_SEC_SEP_BIN

sample protoype for primary secondary application compiled seperately. Primary creates rings, bufferes, intializes the ports and does RX on the traffic.
The secondary process does lookup on the rings and buffers shared, to process the pkts shared.

This is skeleton framework to build any application which requires seperate pid for processing. 
Simple use case is 
1) primary application process ipv4 and ipv6 while Secondary application process unknown packets.
2) primary run openflow agent while secondary runs openflow controller.
3) primary runs suricata dpdk while secondary runs hyperscan accelerated for suspicous packets.

Use cases are infinite. to compile as primary secondary applications
1) create two instances and edit make file (uncomment) for primary and secondary
2) use one folder edit make file after each build.
