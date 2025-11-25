CC = gcc


SRC_DIR = src
BIN_DIR = bin

all: server client

server: $(SRC_DIR)/Server/server_main.c
	$(CC) $(SRC_DIR)/Server/server_main.c -o $(BIN_DIR)/server

client: $(SRC_DIR)/Client/client_main.c
	$(CC) $(SRC_DIR)/Client/client_main.c -o $(BIN_DIR)/client