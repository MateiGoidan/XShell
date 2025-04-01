# Path to the final executable
TARGET = XShell/bin/my_shell

# Source files with relative paths
SRCS = XShell/main.c XShell/xvar.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Default rule
all: $(TARGET)

# Build rule: ensure bin directory exists, then compile
$(TARGET): $(SRCS) | XShell/bin
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Create the bin directory if it doesn't exist
XShell/bin:
	mkdir -p XShell/bin

# Clean the compiled executable
clean:
	rm -f $(TARGET)

# Compile and run the shell
run: all
	./$(TARGET)
