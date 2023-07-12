PWD := $(shell pwd)
obj-m += pemt_module.o
obj-m += ms_module.o
obj-m += cycle_module.o
obj-m += perf_module.o
obj-m += track_module.o
obj-m += overhead_module.o

VERSION=31-1-0
TOOLCHAIN_DIR=gcc-linaro-6.4.1-2017.08-x86_64_aarch64-linux-gnu
CC_PREFIX=aarch64-linux-gnu-
JETPACK=~/pkshin/nvidia_jetpack

CROSS=${HOME}/toolchain_bin_${VERSION}/${TOOLCHAIN_DIR}/bin/${CC_PREFIX}
KERNEL_OUT=${JETPACK}/Xavier/Linux_for_Tegra/sources/kernel/kernel-4.9/build

all:
	make ARCH=arm64 CROSS_COMPILE=$(CROSS) -C $(KERNEL_OUT) M=$(PWD) modules
clean:
	make -C $(KERNEL_OUT) SUBDIRS=$(PWD) clean
