
# LUMS/VORAX Build System
# Complete Makefile with all targets as specified in reports

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -fPIC
DEBUG_FLAGS = -g3 -DDEBUG -fsanitize=address -fsanitize=undefined
RELEASE_FLAGS = -O3 -march=native -DNDEBUG
SRC_DIR = server/lums
BUILD_DIR = build
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
LUMS_LIB = $(BUILD_DIR)/liblums.a
SHARED_LIB = $(BUILD_DIR)/liblums.so

# Default target
all: $(LUMS_LIB)
	@echo "✅ Build completed: $(LUMS_LIB)"

# Static library
$(LUMS_LIB): $(OBJECTS) | $(BUILD_DIR)
	@echo "📦 Creating static library..."
	ar rcs $@ $^
	@echo "✅ Library created: $@"

# Shared library  
shared: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) | $(BUILD_DIR)
	@echo "📦 Creating shared library..."
	$(CC) -shared -o $@ $^
	@echo "✅ Shared library created: $@"

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# C unit tests
test: $(LUMS_LIB)
	@echo "🧪 Running C unit tests..."
	@if [ -f tests/test_lums.c ]; then \
		$(CC) $(CFLAGS) tests/test_lums.c -L$(BUILD_DIR) -llums -o $(BUILD_DIR)/test_lums && \
		./$(BUILD_DIR)/test_lums; \
	else \
		echo "⚠️  No C test file found, skipping C tests"; \
	fi

# JavaScript/TypeScript triple tests
test-js:
	@echo "🧪 Running JavaScript/TypeScript triple tests..."
	npm test

# All tests
test-all: test test-js
	@echo "🎯 All test suites completed successfully"

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(LUMS_LIB)
	@echo "🐛 Debug build completed with AddressSanitizer"

# Release build  
release: CFLAGS += $(RELEASE_FLAGS)
release: clean $(LUMS_LIB)
	@echo "🚀 Release build completed with optimizations"

# Lint C code
lint:
	@echo "🔍 Linting C code..."
	@for file in $(SOURCES); do \
		echo "Checking $$file..."; \
		$(CC) $(CFLAGS) -fsyntax-only $$file || exit 1; \
	done
	@echo "✅ All C files passed lint check"

# Install system-wide
install: $(LUMS_LIB)
	@echo "📥 Installing LUMS library..."
	sudo cp $(LUMS_LIB) /usr/local/lib/
	sudo cp $(SRC_DIR)/lums.h /usr/local/include/
	sudo ldconfig
	@echo "✅ Installation completed"

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -f logs/*.jsonl
	@echo "✅ Clean completed"

# Complete CI pipeline
ci: clean lint test-all
	@echo "🎯 CI Summary:"
	@echo "   ✅ Clean: PASSED"
	@echo "   ✅ Lint: PASSED"  
	@echo "   ✅ Tests: PASSED"
	@echo "   ✅ Build: PASSED"

# Help
help:
	@echo "LUMS/VORAX Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build static library (default)"
	@echo "  shared     - Build shared library"
	@echo "  test       - Run C unit tests"
	@echo "  test-js    - Run JavaScript/TypeScript tests"
	@echo "  test-all   - Run all tests"
	@echo "  debug      - Debug build with sanitizers"
	@echo "  release    - Optimized release build"
	@echo "  lint       - Lint C code"
	@echo "  install    - Install system-wide"
	@echo "  clean      - Clean build artifacts"
	@echo "  ci         - Complete CI pipeline"
	@echo "  help       - Show this help"

# Build information
info:
	@echo "Build Configuration:"
	@echo "  CC: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Library: $(LUMS_LIB)"

.PHONY: all shared test test-js test-all debug release lint install clean ci help info
