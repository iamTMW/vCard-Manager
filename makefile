#Name: Taha Mohyuddin
#Student ID: 1275575 
#Date: 9 - Mar - 2025
#Course: CIS2750

CC = gcc

# Below are the Directories
INCLUDE_DIR = include
SRC_DIR = src
BIN_DIR = bin

# Below are the flags
CFLAGS = -Wall -g -fPIC -I$(INCLUDE_DIR)
LDFLAGS = -shared


SRC_FILES = \
	$(SRC_DIR)/VCParser.c \
	$(SRC_DIR)/VCHelpers.c \
	$(SRC_DIR)/VCWrapper.c \
	$(SRC_DIR)/LinkedListAPI.c

OBJ_FILES = $(SRC_FILES:.c=.o)


TARGET_LIB = $(BIN_DIR)/libvcparser.so


all: parser

parser: $(BIN_DIR) $(TARGET_LIB)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET_LIB): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_FILES)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET_LIB)