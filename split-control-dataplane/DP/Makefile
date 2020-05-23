
ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = dp

S_DPDK_DIR = $(shell pwd)
CM_DIR = $(S_DPDK_DIR)/../../CM
CM_INC = $(S_DPDK_DIR)/../../CM/inc
CM_LIB = $(S_DPDK_DIR)/../../CM/lib/libcparser.a
CM_SRC = $(CM_DIR)/action.c  $(CM_DIR)/api.c $(CM_DIR)/config.c $(CM_DIR)/cparser_tree.c $(CM_DIR)/ssli_parser.c

# all source are stored in SRCS-y
SRCS-y := $(CM_SRC) dpcommon.c frameparser.c l2parser.c main.c

CFLAGS += -O3
#CFLAGS += $(WERROR_FLAGS)
CFLAGS += -I $(CM_INC) -I $(CM_DIR)
LDFLAGS += $(CM_LIB)

include $(RTE_SDK)/mk/rte.extapp.mk

