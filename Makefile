CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude -g
LDFLAGS = -pthread

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Common sources
COMMON_SRCS = $(wildcard $(SRC_DIR)/common/*.c)
COMMON_OBJS = $(patsubst $(SRC_DIR)/common/%.c, $(OBJ_DIR)/common/%.o, $(COMMON_SRCS))

# Server sources
SERVER_SRCS = $(wildcard $(SRC_DIR)/Server/*.c)
SERVER_OBJS = $(patsubst $(SRC_DIR)/Server/%.c, $(OBJ_DIR)/Server/%.o, $(SERVER_SRCS))

# Client sources
CLIENT_SRCS = $(wildcard $(SRC_DIR)/Client/*.c)
CLIENT_OBJS = $(patsubst $(SRC_DIR)/Client/%.c, $(OBJ_DIR)/Client/%.o, $(CLIENT_SRCS))

# Targets
all: directories Server Client

Server: $(COMMON_OBJS) $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/Server $^

Client: $(COMMON_OBJS) $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/Client $^

# Compile common objects
$(OBJ_DIR)/common/%.o: $(SRC_DIR)/common/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile server objects
$(OBJ_DIR)/Server/%.o: $(SRC_DIR)/Server/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile client objects
$(OBJ_DIR)/Client/%.o: $(SRC_DIR)/Client/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

directories:
	@mkdir -p $(OBJ_DIR)/common $(OBJ_DIR)/Server $(OBJ_DIR)/Client $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories server client
