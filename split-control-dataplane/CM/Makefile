CC=gcc
WORKING_DIR=$(shell pwd)
SRC_PATH=$(WORKING_DIR)
SOURCE =cparser_tree.c ssli_parser.c action.c config.c api.c
INC_PATH=$(WORKING_DIR)/inc
LIB_PATH=$(WORKING_DIR)/lib

all:
	$(CC)  $(SOURCE) -I$(INC_PATH) $(LIB_PATH)/libcparser.a -o cli

clean:
	rm cli 
