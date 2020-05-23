# DPDK-LINK-DIAG
simple app running in primary and secondary instance polling for link states

To Start:
Install desired DPDK instance.
Build for desired target
Bind uio driver with supported NIC.
Create huge pages are required. 
Run application with "<appname> -l <core> -n 3 --process=auto -m 8" (as primary and secondary)
