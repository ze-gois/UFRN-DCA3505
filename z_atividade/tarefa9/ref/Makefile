# Makefile for scheduling experiment

# Compiler settings
CC = gcc
CFLAGS = -Wall -pthread

# Target executable
TARGET = spawn

# Source files
SRCS = spawn.c

# Default target
all: clean $(TARGET) run

# Compile the program
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Run the experiment
run:
	./$(TARGET)

# Clean up
clean:
	rm -rf $(TARGET) ./log/ps-*.log || true
	mkdir -p ./log

# Target to show logs
logs:
	ls -la ./log/

.PHONY: all run clean logs