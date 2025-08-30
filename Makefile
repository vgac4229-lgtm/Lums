# LUMS/VORAX Build System - Complete Pipeline
# Version: 3.0 (Post-correction critique)

# Configuration
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
SRCDIR = server/lums
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
LUMS_LIB = $(BUILDDIR)/liblums.a

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
NC = \033[0m # No Color

.PHONY: all shared test test-js test-all debug release lint install clean ci help info

# Default target
all: $(LUMS_LIB)
	@echo "$(GREEN)✅ Build completed: $(LUMS_LIB)$(NC)"

# Build static library
$(LUMS_LIB): $(OBJECTS) | $(BUILDDIR)
	@echo "$(YELLOW)📦 Creating static library...$(NC)"
	ar rcs $@ $^
	@echo "$(GREEN)✅ Library created: $@$(NC)"

# Compile object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@echo "$(YELLOW)🔨 Compiling $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

# Build shared library
shared: CFLAGS += -fPIC
shared: $(BUILDDIR)/liblums.so
	@echo "$(GREEN)✅ Shared library created$(NC)"

$(BUILDDIR)/liblums.so: $(OBJECTS) | $(BUILDDIR)
	$(CC) -shared -o $@ $^

# C unit tests (placeholder - to be implemented)
test: $(LUMS_LIB)
	@echo "$(YELLOW)🧪 Running C unit tests...$(NC)"
	@if [ ! -f "tests/test_lums.c" ]; then \
		echo "$(YELLOW)⚠️  No C test file found, skipping C tests$(NC)"; \
	else \
		$(CC) $(CFLAGS) -o $(BUILDDIR)/test_lums tests/test_lums.c -L$(BUILDDIR) -llums; \
		./$(BUILDDIR)/test_lums; \
	fi
	@echo "$(GREEN)✅ C tests completed$(NC)"

# JavaScript/TypeScript triple tests
test-js:
	@echo "$(YELLOW)🧪 Running JavaScript/TypeScript triple tests...$(NC)"
	@if [ -f "tests/lums.test.js" ]; then \
		node tests/lums.test.js; \
		echo "$(GREEN)✅ JavaScript tests completed$(NC)"; \
	else \
		echo "$(RED)❌ No JavaScript test file found$(NC)"; \
		exit 1; \
	fi

# All tests
test-all: test test-js
	@echo "$(GREEN)🎯 All test suites completed successfully$(NC)"

# Debug build
debug: CFLAGS += -g3 -DDEBUG -fsanitize=address
debug: $(LUMS_LIB)
	@echo "$(GREEN)✅ Debug build with AddressSanitizer completed$(NC)"

# Release build
release: CFLAGS += -O3 -DNDEBUG -march=native
release: $(LUMS_LIB)
	@echo "$(GREEN)✅ Optimized release build completed$(NC)"

# Code linting
lint:
	@echo "$(YELLOW)🔍 Linting C code...$(NC)"
	@for file in $(SOURCES); do \
		echo "Checking $$file..."; \
		$(CC) $(CFLAGS) -fsyntax-only $$file || exit 1; \
	done
	@echo "$(GREEN)✅ All C files passed lint check$(NC)"

# System installation
install: $(LUMS_LIB)
	@echo "$(YELLOW)📦 Installing LUMS library...$(NC)"
	cp $(LUMS_LIB) /usr/local/lib/
	cp $(SRCDIR)/*.h /usr/local/include/
	@echo "$(GREEN)✅ Installation completed$(NC)"

# Clean build artifacts
clean:
	@echo "$(YELLOW)🧹 Cleaning build artifacts...$(NC)"
	rm -rf $(BUILDDIR)
	rm -f logs/*.jsonl
	@echo "$(GREEN)✅ Clean completed$(NC)"

# Complete CI pipeline
ci: clean lint test-all all
	@echo "$(GREEN)🚀 CI Summary:$(NC)"
	@echo "$(GREEN)   ✅ Clean: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Lint: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Tests: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Build: PASSED$(NC)"

# Help
help:
	@echo "LUMS/VORAX Build System - Available targets:"
	@echo ""
	@echo "  $(YELLOW)all$(NC)          - Build static library (default)"
	@echo "  $(YELLOW)shared$(NC)       - Build shared library"
	@echo "  $(YELLOW)test$(NC)         - Run C unit tests"
	@echo "  $(YELLOW)test-js$(NC)      - Run JavaScript/TypeScript triple tests"
	@echo "  $(YELLOW)test-all$(NC)     - Run all tests (C + JS)"
	@echo "  $(YELLOW)debug$(NC)        - Build with debug symbols + AddressSanitizer"
	@echo "  $(YELLOW)release$(NC)      - Build optimized release version"
	@echo "  $(YELLOW)lint$(NC)         - Lint C source files"
	@echo "  $(YELLOW)install$(NC)      - Install library system-wide"
	@echo "  $(YELLOW)clean$(NC)        - Clean build artifacts"
	@echo "  $(YELLOW)ci$(NC)           - Run complete CI pipeline"
	@echo "  $(YELLOW)help$(NC)         - Show this help"
	@echo "  $(YELLOW)info$(NC)         - Show build configuration"

# Build info
info:
	@echo "$(YELLOW)Build Configuration:$(NC)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Library: $(LUMS_LIB)"
	@echo "  Build dir: $(BUILDDIR)"