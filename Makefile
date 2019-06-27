Q := @
CC := gcc
CVERSION := -std=c99


INCLUDE_DIRS := -Iinclude -Ilib/jsonschema-c -Isrc/utils -I/usr/include/json-c
SOURCE_DIR := src
OBJECT_DIR := obj
BINARY_DIR := bin

CFLAGS := $(INCLUDE_DIRS) -g 
LFLAGS := -lpthread -ldl -ljson-c -lmiddleware_api -lmiddleware_utils -lncursesw

C_FILES := $(wildcard $(SOURCE_DIR)/*.c)
O_FILES := $(addprefix $(OBJECT_DIR)/,$(notdir $(C_FILES:.c=.o)))
B_FILES := $(addprefix $(BINARY_DIR)/,$(notdir $(C_FILES:.c=.out)))


all: $(B_FILES)


$(BINARY_DIR)/%.out: $(OBJECT_DIR)/%.o
	@echo "	LD $@"
	$(Q)mkdir -p $(BINARY_DIR)
	$(Q)$(CC) $< -o $(BINARY_DIR)/$(notdir $@) $(LFLAGS)


$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	@echo "	CC $<"
	$(Q)mkdir -p $(OBJECT_DIR)
	$(Q)$(CC) -c $< -o $@ $(CFLAGS)

