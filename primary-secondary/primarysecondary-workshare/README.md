# DPDK_PRIMARY_SECONDARY_WORKSHARE

## Motivation
 - Allow to run muliple application on unique packet signatures.
 - Allow development team to work on indiviudal code base without depending upon the Primary.

## Scope 
 - Packet classification is done by either Primary|Flow Director to mapped RX queues.
 - bifurcated traffic are consumed by secondary applciation on mapped rx-tx quues

## Purpose

 - Sample program to show case use of primary secondary process in DPDK for packet processing.
 - Use case scenrario can be exception traffic to be processed on the secondary while primary takes care of main datapath.

## Usage
```
Primary: ./build/test -n 4 -l 10-16 -d <dpdk sdk path>/x86_64-native-linuxapp-gcc/lib/librte_pmd_i40e.so  --proc-type=auto
Secondary: ./build/test -n 4 -l 18-22  --proc-type=auto -d <dpdk sdk path>/x86_64-native-linuxapp-gcc/lib/librte_pmd_i40e.so
```

note: 
* running on shared lib hence -d option for ethernet device detection is present.
* to do: build packet datapath processing, currently all packets are excpetion category
