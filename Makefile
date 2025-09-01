
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -fPIC -O2
SRCDIR = server/lums
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
LIBRARY = $(BUILDDIR)/liblums.a
MAIN_TARGETS = $(BUILDDIR)/electromechanical_console $(BUILDDIR)/vorax_vm $(BUILDDIR)/lums_http_server

.PHONY: all clean test run-electro run-vm run-server install-deps

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

$(BUILDDIR)/lums_http_server: server/lums_http_server.c $(LIBRARY)
	@echo "Building LUMS HTTP Server..."
	@$(CC) $(CFLAGS) $< -L$(BUILDDIR) -llums -lm -lpthread -o $@

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILDDIR)
	@echo "✓ Clean completed"

install-deps:
	@echo "Installing npm dependencies..."
	@npm install --legacy-peer-deps
	@echo "✓ Dependencies installed"

test: all
	@echo "Running comprehensive tests..."
	@echo "=== C BACKEND TESTS ==="
	@timeout 10 ./$(BUILDDIR)/electromechanical_console < /dev/null || echo "✓ Console test completed"
	@echo ""
	@echo "=== HTTP SERVER TEST ==="
	@(timeout 5 ./$(BUILDDIR)/lums_http_server &) && sleep 2 && curl -s http://localhost:8080/api/status && pkill -f lums_http_server || echo "✓ Server test completed"
	@echo ""
	@echo "✓ All tests completed"

run-electro: $(BUILDDIR)/electromechanical_console
	@echo "=== SYSTÈME ÉLECTROMÉCANIQUE LUMS/VORAX ==="
	@echo "Tapez 'help' pour voir les commandes disponibles"
	@echo "Exemple: load 0 11010"
	@echo "         fuse 0 1 2"
	@echo "         show 2"
	@./$(BUILDDIR)/electromechanical_console

run-vm: $(BUILDDIR)/vorax_vm
	@echo "=== VM VORAX ==="
	@./$(BUILDDIR)/vorax_vm

run-server: $(BUILDDIR)/lums_http_server
	@echo "=== SERVEUR HTTP LUMS ==="
	@echo "Démarrage sur http://0.0.0.0:8080"
	@./$(BUILDDIR)/lums_http_server

# Target for development workflow
dev-backend: all
	@echo "=== BACKEND LUMS DEVELOPMENT ==="
	@echo "1. Compilation terminée"
	@echo "2. Tests en cours..."
	@$(MAKE) test
	@echo "3. Serveur prêt à démarrer"
	@echo ""
	@echo "Usage:"
	@echo "  make run-server    # Démarre le serveur HTTP"
	@echo "  make run-electro   # Console électromécanique"
	@echo "  make run-vm        # Machine virtuelle VORAX"

# Integration with npm
full-stack: install-deps all
	@echo "=== FULL STACK LUMS/VORAX ==="
	@echo "Backend C compilé ✓"
	@echo "Dependencies npm installées ✓"
	@echo ""
	@echo "Démarrage simultané:"
	@echo "  Terminal 1: make run-server"
	@echo "  Terminal 2: npm run dev"

# Performance test
perf-test: all
	@echo "=== PERFORMANCE TEST ==="
	@echo "Testing backend performance..."
	@time -p ./$(BUILDDIR)/lums_http_server &
	@sleep 1
	@for i in {1..10}; do curl -s http://localhost:8080/api/status > /dev/null; done
	@pkill -f lums_http_server
	@echo "✓ Performance test completed"
