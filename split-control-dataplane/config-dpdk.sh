dpdkPath="/home/acceluser/Documents/Projects/IDS_DPDK/dpdk-1.8.0/"

echo "Starting to configure"
echo "DPDK: ${dpdkPath}"

modprobe uio
insmod ${dpdkPath}build/kmod/igb_uio.ko
${dpdkPath}tools/dpdk_nic_bind.py --bind=igb_uio $(${dpdkPath}tools/dpdk_nic_bind.py --status | sed -rn 's,.* if=([^ ]*).*igb_uio *$,\1,p')
mount -t hugetlbfs nodev /mnt/huge
echo 64 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
for gov in /sys/devices/system/cpu/*/cpufreq/scaling_governor ; do echo performance >$gov ; done

echo "Due export these to terminal !!"
echo "export RTE_SDK=${dpdkPath}"
echo "export RTE_TARGET=x86_64-native-linuxapp-gcc"

echo "Configured !!"
