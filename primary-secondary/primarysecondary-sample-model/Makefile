ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overridden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = primary
#APP = secondary

# all source are stored in SRCS-y
SRCS-y := node.c stats.c main_primary.c
#SRCS-y := node.c stats.c main_secondary.c

CFLAGS += $(WERROR_FLAGS)

# workaround for a gcc bug with noreturn attribute
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=12603
ifeq ($(CONFIG_RTE_TOOLCHAIN_GCC),y)
CFLAGS_main.o += -Wno-return-type
endif

EXTRA_CFLAGS += -O2 -g -Wfatal-errors

include $(RTE_SDK)/mk/rte.extapp.mk
