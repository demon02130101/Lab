# Compiler and compile flags
CC = g++
CFLAGS = -std=c++11

# Executable name
TARGET = Lab1

# Source files
SRCS = Lab1.cpp

# Default target: build the executable
all: $(TARGET)

# Rule to link object files into the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up compiled files
clean:
	rm -f $(TARGET)
