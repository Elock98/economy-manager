# Compiler and flags
CXX := g++
CXXFLAGS := `wx-config --cxxflags` -Wall -std=c++17 -I. -g # Add -I. to include the current directory
LDFLAGS := `wx-config --libs`

# Directories
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := build

# Output binary
TARGET := $(BIN_DIR)/EconoManager

# Source files
SRC := $(SRC_DIR)/main.cpp
OBJ := $(OBJ_DIR)/main.o

# Rules for building
.PHONY: all clean

all: $(TARGET)

# Create binary directory and link the object file to create the executable
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -g

# Compile Editor
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/main.h
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)