# Makefile pour LUMS/VORAX Électromécanique
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g
SRCDIR = server/lums
BUILDDIR = build
TARGET_LIB = $(BUILDDIR)/liblums.a
TARGET_CONSOLE = $(BUILDDIR)/electromechanical_console

# Sources
SOURCES = $(SRCDIR)/encoder.c $(SRCDIR)/decoder.c $(SRCDIR)/operations.c $(SRCDIR)/vorax.c $(SRCDIR)/electromechanical.c
CONSOLE_SRC = $(SRCDIR)/electromechanical_console.c
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
CONSOLE_OBJ = $(CONSOLE_SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Rules
.PHONY: all clean test install

all: $(TARGET_LIB) $(TARGET_CONSOLE)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_LIB): $(OBJECTS)
	@echo "Creating static library..."
	ar rcs $@ $^
	@echo "✓ Library created: $@"

$(TARGET_CONSOLE): $(CONSOLE_OBJ) $(TARGET_LIB)
	@echo "Building electromechanical console..."
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -llums -o $@
	@echo "✓ Console executable created: $@"

clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILDDIR)
	@echo "✓ Clean completed"

test: $(TARGET_CONSOLE)
	@echo "Running electromechanical system tests..."
	@echo "test" | $(TARGET_CONSOLE)
	@echo "✓ Tests completed"

install: all
	@echo "✓ LUMS/VORAX Électromécanique ready"
	@echo "Run: ./$(TARGET_CONSOLE)"

# Development targets
dev: all
	@echo "=== DÉVELOPPEMENT ÉLECTROMÉCANIQUE ==="
	@echo "Console: ./$(TARGET_CONSOLE)"
	@echo "Tests:   make test"

# Production build
production: CFLAGS += -O3 -DNDEBUG
production: clean all
	@echo "✓ Production build completed"

# Help
help:
	@echo "LUMS/VORAX Électromécanique Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build library and console"
	@echo "  clean       - Remove build files"
	@echo "  test        - Run system tests"
	@echo "  install     - Build and prepare for use"
	@echo "  dev         - Development build with info"
	@echo "  production  - Optimized production build"
	@echo "  help        - Show this help"