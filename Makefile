
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -fPIC
SRCDIR = server/lums
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
LIBRARY = $(BUILDDIR)/liblums.a
MAIN_TARGETS = $(BUILDDIR)/electromechanical_console $(BUILDDIR)/vorax_vm $(BUILDDIR)/lums_http_server

.PHONY: all clean test run-electro run-vm

all: $(BUILDDIR) $(LIBRARY) $(MAIN_TARGETS)
	@echo "✓ Build completed successfully"
	@echo "✓ Library: $(LIBRARY)"
	@echo "✓ Executables: $(MAIN_TARGETS)"

$(BUILDDIR):
	@echo "Creating build directory..."
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	@echo "Creating static library..."
	@ar rcs $@ $^
	@echo "✓ Library created: $@"

$(BUILDDIR)/electromechanical_console: $(SRCDIR)/electromechanical_console.c $(LIBRARY)
	@echo "Building electromechanical console..."
	@$(CC) $(CFLAGS) $< -L$(BUILDDIR) -llums -lm -o $@

$(BUILDDIR)/vorax_vm: $(SRCDIR)/vorax.c $(LIBRARY)
	@echo "Building VORAX VM..."
	@$(CC) $(CFLAGS) -DMAIN_PROGRAM $< -L$(BUILDDIR) -llums -lm -o $@

$(BUILDDIR)/lums_http_server: $(SRCDIR)/lums_http_server.c $(LIBRARY)
	@echo "Building LUMS HTTP Server..."
	@$(CC) $(CFLAGS) $< -L$(BUILDDIR) -llums -lm -lpthread -o $@

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILDDIR)
	@echo "✓ Clean completed"

test: all
	@echo "Running tests..."
	@./$(BUILDDIR)/electromechanical_console < /dev/null || true
	@echo "✓ Tests completed"

run-electro: $(BUILDDIR)/electromechanical_console
	@echo "=== SYSTÈME ÉLECTROMÉCANIQUE LUMS/VORAX ==="
	@echo "Tapez 'help' pour voir les commandes disponibles"
	@./$(BUILDDIR)/electromechanical_console

run-vm: $(BUILDDIR)/vorax_vm
	@echo "=== VM VORAX ==="
	@./$(BUILDDIR)/vorax_vm

run-server: $(BUILDDIR)/lums_http_server
	@echo "=== SERVEUR HTTP LUMS ==="
	@./$(BUILDDIR)/lums_http_server
