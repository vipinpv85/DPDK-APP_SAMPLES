# DPDK-INTERCEPT
DPDK Based EAST-WEST packet processing &amp; distributor logic 

# Bottleneck for Scalability

Running CP and DP, has to have dedicated cores. Difficult to achieve true balance among them
Running multiple instance of CP+DP has overhead of CP and route table synchronization.
SDN centralized helps to bifurcate the need for CP+DP but does not address or scale the DP (only modularizes).
DP can be demanding and needs to scale up or down as when required.
Expensive to build or procure Custom ASIC or DP processing modules.

## SOLUTIONS:
Split the DP and CP physically and logically
Employ virtualization with orchestration to scale up or down based on load.
Cluster similar instances of NFV of together.
Allow resource or memory table sharing, to reduce the memory foot print.
