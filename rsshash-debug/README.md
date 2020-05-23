# RSS_HASH_DEBUG-via-TAP

## Motivation
 - DPDK NIC supporting RSS allows to distribute incoming traffic to multiple RX queue.
 - Packet drops are found when user application consumes the incoming traffic and create replies.
 - Isolating such scenarios with 3 or 5 tuple RSS is difficult.
 - In case of VF NIC with PF filters based on RSS it is diffcult to isolate the issue.
 
## Solution:
Create per port-queue or port NIC map to kernel (via **KNI or TAP**) or user applciation

## Steps
- Map each NIC (VF|PF) rx queues with a tap interface.
- Assign ip address to tap interfaces.
- Enabe promisc mode for tap for recieving all packets (optional).
- optionally move them to seperate work space for further debug
- Run traffic at NIC itnerface level.
- Terminate the application to disaply the stats to compare.
