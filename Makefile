# =============================================================================
# Ice Configuration Parser - Makefile
# =============================================================================

# Project configuration
PROJECT=icelang
RULESDEF=src/IceApi.re
GENERATED=gen/IceApi.cpp
EXE=dist/ice
TEST_EXE=dist/ice-tests
CC=g++ -g
LEXER=re2c
CFLAGS=-std=c++17 -Wall -Wextra
BUILD_DIR=build
GEN_DIR=gen
DIST_DIR=dist

# Try to find lemon in common locations
LEMON=$(shell which lemon 2>/dev/null || echo "lemon")
LEMON_PAR=$(shell find /opt/homebrew -name "lempar.c" 2>/dev/null | head -1 || find /usr -name "lempar.c" 2>/dev/null | head -1 || find /opt -name "lempar.c" 2>/dev/null | head -1 || echo "/usr/share/lemon/lempar.c")

# Source files
SOURCES = src/StrUtil.cpp src/Nodes.cpp src/DebugParser.cpp src/IceParser.cpp src/ConditionalResolver.cpp src/ReferenceResolver.cpp src/Resolver.cpp src/MathResolver.cpp src/CallableDescriptorBuilder.cpp src/DocumentBuilder.cpp
OBJECTS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES)) $(BUILD_DIR)/IceApi.o $(BUILD_DIR)/icelang.o
TEST_SOURCES = $(shell find tests -name '*.cpp' | sort)

# =============================================================================
# Build Rules
# =============================================================================

# Compile object files
$(BUILD_DIR)/%.o: src/%.cpp
	@echo "Compiling $<..."
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ -I$(GEN_DIR) -Isrc

$(BUILD_DIR)/IceApi.o: $(GEN_DIR)/IceApi.cpp
	@echo "Compiling IceApi..."
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ -I$(GEN_DIR) -Isrc -I.

$(BUILD_DIR)/icelang.o: $(GEN_DIR)/icelang.c $(GEN_DIR)/icelang.h
	@echo "Compiling parser..."
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ -I$(GEN_DIR) -Isrc

# =============================================================================
# Code Generation Rules
# =============================================================================

# Generate parser
$(GEN_DIR)/icelang.c $(GEN_DIR)/icelang.h: src/icelang.y
	@echo "Generating parser..."
	mkdir -p $(GEN_DIR)
	@if [ ! -f $(GEN_DIR)/icelang.c ]; then \
		echo "Generating parser with lemon..."; \
		$(LEMON) -T$(LEMON_PAR) -d$(GEN_DIR) $<; \
	else \
		echo "Using existing parser files..."; \
	fi

# Generate lexer
$(GEN_DIR)/IceApi.cpp: $(RULESDEF) $(GEN_DIR)/icelang.h
	@echo "Generating lexer..."
	mkdir -p $(GEN_DIR)
	$(LEXER) -ci -o $@ $< --tags

# Create header dependency
$(GEN_DIR)/icelang.h: $(GEN_DIR)/icelang.c

# =============================================================================
# Main Targets
# =============================================================================

# Build everything (clean + lib + cli)
all: clean lib cli
	@echo "Build complete! 🎉"

# Build the library
lib: $(DIST_DIR)/libice.a
	@echo "Library built: $(DIST_DIR)/libice.a"

# Build the CLI executable
cli: $(DIST_DIR)/ice
	@echo "CLI built: $(DIST_DIR)/ice"

# Build and run the CLI with the default test file
exec: cli
	./$(EXE) -f test.ice

# Build and run parser behavior tests
test: $(TEST_EXE)
	./$(TEST_EXE)

# =============================================================================
# Library and Executable Rules
# =============================================================================

# Build the library
$(DIST_DIR)/libice.a: $(OBJECTS)
	@echo "Creating library..."
	mkdir -p $(DIST_DIR)
	ar rcs $@ $^

# Build the CLI executable
$(DIST_DIR)/ice: $(DIST_DIR)/libice.a
	@echo "Building CLI executable..."
	mkdir -p $(DIST_DIR)
	$(CC) $(CFLAGS) -o $@ src/main.cpp $(DIST_DIR)/libice.a -I$(GEN_DIR) -Isrc

# Build the test executable
$(TEST_EXE): $(DIST_DIR)/libice.a $(TEST_SOURCES) tests/doctest.hpp tests/TestSupport.hpp
	@echo "Building test executable..."
	mkdir -p $(DIST_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SOURCES) $(DIST_DIR)/libice.a -I$(GEN_DIR) -Isrc -Itests

# =============================================================================
# Clean Targets
# =============================================================================

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)/*
	rm -rf $(DIST_DIR)/*
	rm -rf $(GEN_DIR)/*

# =============================================================================
# Help Target
# =============================================================================

help:
	@echo "Ice Configuration Parser - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all     - Clean and build everything (library + CLI)"
	@echo "  lib     - Build the static library (dist/libice.a)"
	@echo "  cli     - Build the CLI executable (dist/ice)"
	@echo "  exec    - Build and run dist/ice with test.ice"
	@echo "  test    - Build and run parser behavior tests"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"
	@echo ""

.PHONY: all lib cli exec test build main clean distclean help
