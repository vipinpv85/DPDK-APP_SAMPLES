rm -rf version.h build/*

sdk=${RTE_SDK}
target=${RTE_TARGET}

echo " #define UNAME \"`uname -a`\"" > version.h
echo " #define DPDK_SDK \"${sdk}\"" >> version.h
echo " #define DPDK_TARGET \"${target}\"" >> version.h
echo " #define GCC_VER \"`gcc -v 2>&1|tail -1`\"" >> version.h

make clean; make
