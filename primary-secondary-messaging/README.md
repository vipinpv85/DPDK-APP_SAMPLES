# Primary-Secondary event driven workshare

## Motivation
Applications can be modeled to work in primary distributor to secondary worker due to

- let primary handle all resource allocation and configuration.
- secondary can be spawned up-down with no overhead (dynamically scale up-down) 
- faster worker logic for context switch
- isolation and easy porting to existing application for scaling on cores.

In all above scenarios we are requure huge page shared memory area were task are shared via ring. 
For worker to retrive such task continous task from ring will.lead to 100% polling, which leads to nom dpdk thread to context switch.
Using delayed polled loop, still puts the core to run at 100%, so use `usleep(10)` in worker (secondary).

## Purpose
If continous poll can be converted to asynchronous event driven model, this would be a remedy to the problem


## Status
explore

## How it works
use DPDK mp thread to intimate or communicate events between primary and secondary

## How to Run

* Primary: ./build/helloworld -c 0x10 --proc-type=primary
* Secondary:  ./build/helloworld -c 0x100  --proc-type=secondary
