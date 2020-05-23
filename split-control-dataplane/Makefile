
ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = xcelS

S_DPDK_DIR = $(shell pwd)
CM_DIR = $(S_DPDK_DIR)/../CM
DP_DIR = $(S_DPDK_DIR)/../DP
CM_INC = $(CM_DIR)/inc

CM_LIB = $(CM_DIR)/lib/libcparser.a

CM_SRC = $(CM_DIR)/action.c\
         $(CM_DIR)/api.c\
         $(CM_DIR)/config.c\
         $(CM_DIR)/cparser_tree.c\
         $(CM_DIR)/ssli_parser.c

DP_SRC = $(DP_DIR)/userRoutine.c\
         $(DP_DIR)/l4parser.c\
         $(DP_DIR)/l3parser.c\
         $(DP_DIR)/dpapi.c\
         $(DP_DIR)/dptables.c\
         $(DP_DIR)/dpcommon.c\
         $(DP_DIR)/frameparser.c\
         $(DP_DIR)/l2parser.c\
         $(DP_DIR)/main.c

# all source are stored in SRCS-y
SRCS-y := $(CM_SRC) $(DP_SRC)

CFLAGS += -O2
#CFLAGS += $(WERROR_FLAGS)
CFLAGS += -I $(CM_INC) -I $(CM_DIR) -I $(DP_DIR)
LDFLAGS += $(CM_LIB)

include $(RTE_SDK)/mk/rte.extapp.mk

