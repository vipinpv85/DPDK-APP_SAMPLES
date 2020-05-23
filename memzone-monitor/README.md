# DPDK-MEMZONEMONITOR

![memzone](https://user-images.githubusercontent.com/1296097/62437377-83f31480-b760-11e9-8bf8-8b18b6dd6c13.png)

## Motivation
DPDK applications can be modeled with single run to completion or multi pipe line stage completion models. Single or multiple process can itnerac twith packets and data alike. Hence corruption in any of the data, lookup tabels or counters in hard to isolate. This can occur when certan excpetion code process is taken hence diffcult to reproduce too.

## Solution
We can isolate above scenarios if low overhead process monitors changes occuring in a table or counters which is not expected to update. Using DPDK multi-process model we can get access to all huge page content from primary or othe rmulti process applciations

## what it is?
DPDK secondary application to monitor the memzone region for updates or changes.

## The application helps to 
- periodically montior memzone zone
- hash is performed for (total length/CACHELINE length).
- HASH can be set to CRC or JHASH. 
- Changes are compared continously.
- Optoin to write to log file is added
