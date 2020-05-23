# DPDK-EBPF-DEBUG

## Motivation
Create a simplified tracer with dynamic data probe which allows to collect, act and account for user or DPDK library events.

![ebpf-debug-tool](https://user-images.githubusercontent.com/1296097/65375432-37817b00-dcb3-11e9-9b50-29325b386fda.png)

## Things to do
- [x] Explore DPDK examples for trace-debug scenarios
- [x] Add CSV trace generation with time stamps.
- [x] Sample python matplot for data reprenstation.
- [ ] Convert static trace to userProbe like dynamic trace.

## How to Run?

- Download DPDK from dpdk.org.
- Untar DPDK tar file.
- Execute the following commands

```
 cd <to unatar dpdk folder>
 make config T=x86_64-native-linuxapp-gcc O=x86_64-native-linuxapp-gcc
 export RTE_SDK=$PWD
 export RTE_TARGET=x86_64-native-linuxapp-gcc
 cd x86_64-native-linuxapp-gcc
 vim .config
 set CONFIG_RTE_LIBRTE_BPF and CONFIG_RTE_LIBRTE_BPF_ELF to 'y'
 make -j 4
 git checkout or unzip DPDK-EBPF-DEBUG
 cd DPDK-EBPF-DEBUG
 cd to desired applciation
 make -j 4
 cd bpf
 clang -O2 -U __GNUC__ -target bpf -c <desired bpf program> -I$RTE_SDK/$RTE_TARGET/include -L $RTE_SDK/$RTE_TARGET/lib/ $RTE_SDK/$RTE_TARGET/lib/libdpdk.a
```
