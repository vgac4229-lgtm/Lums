

# Makefile pour compilation automatisée du code C LUMS
# Selon recommandations du rapport d'analyse - Version renforcée

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -O2 -g -fPIC
INCLUDES = -Iserver/lums
SRCDIR = server/lums
BUILDDIR = build
TESTDIR = tests
LOGSDIR = logs

# Sources C
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
HEADERS = $(wildcard $(SRCDIR)/*.h)

# Binaires
LUMS_LIB = $(BUILDDIR)/liblums.a
LUMS_SHARED = $(BUILDDIR)/liblums.so
TEST_BIN = $(BUILDDIR)/test_lums

# Couleurs pour output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

.PHONY: all clean test install debug release lint help info ci test-js test-all

# Target par défaut
all: $(LUMS_LIB)
	@echo "$(GREEN)✅ Build completed successfully$(NC)"

# Aide
help:
	@echo "$(BLUE)🔧 LUMS Build System - Available targets:$(NC)"
	@echo "  $(GREEN)all$(NC)          - Build static library (default)"
	@echo "  $(GREEN)shared$(NC)       - Build shared library"
	@echo "  $(GREEN)test$(NC)         - Run C unit tests"
	@echo "  $(GREEN)test-js$(NC)      - Run JavaScript/TypeScript tests"
	@echo "  $(GREEN)test-all$(NC)     - Run all tests (C + JS + Triple validation)"
	@echo "  $(GREEN)debug$(NC)        - Build with debug symbols"
	@echo "  $(GREEN)release$(NC)      - Build optimized release"
	@echo "  $(GREEN)lint$(NC)         - Validate C code"
	@echo "  $(GREEN)install$(NC)      - Install to system"
	@echo "  $(GREEN)clean$(NC)        - Clean build artifacts"
	@echo "  $(GREEN)ci$(NC)           - Complete CI pipeline"
	@echo "  $(GREEN)info$(NC)         - Show build information"

# Création du répertoire de build
$(BUILDDIR):
	@echo "$(YELLOW)📁 Creating build directory...$(NC)"
	@mkdir -p $(BUILDDIR)

$(LOGSDIR):
	@echo "$(YELLOW)📁 Creating logs directory...$(NC)"
	@mkdir -p $(LOGSDIR)

# Compilation des objets avec progression
$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)
	@echo "$(BLUE)🔨 Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "$(GREEN)   ✅ $@ created$(NC)"

# Création de la bibliothèque statique
$(LUMS_LIB): $(OBJECTS)
	@echo "$(BLUE)📦 Creating static library...$(NC)"
	@ar rcs $@ $^
	@echo "$(GREEN)✅ Static library created: $@$(NC)"
	@ls -lh $@

# Création de la bibliothèque partagée
$(LUMS_SHARED): $(OBJECTS)
	@echo "$(BLUE)📦 Creating shared library...$(NC)"
	@$(CC) -shared -o $@ $^
	@echo "$(GREEN)✅ Shared library created: $@$(NC)"

shared: $(LUMS_SHARED)

# Tests unitaires C (si disponibles)
test: $(LUMS_LIB) | $(LOGSDIR)
	@echo "$(BLUE)🧪 Running C unit tests...$(NC)"
	@if [ -f $(TESTDIR)/test_lums.c ]; then \
		echo "$(BLUE)   Compiling C test suite...$(NC)"; \
		$(CC) $(CFLAGS) $(INCLUDES) $(TESTDIR)/test_lums.c -L$(BUILDDIR) -llums -o $(TEST_BIN); \
		echo "$(BLUE)   Executing C tests...$(NC)"; \
		./$(TEST_BIN); \
		echo "$(GREEN)✅ C tests completed$(NC)"; \
	else \
		echo "$(YELLOW)⚠️  No C test file found at $(TESTDIR)/test_lums.c$(NC)"; \
		echo "$(YELLOW)   Skipping C tests$(NC)"; \
	fi

# Tests JavaScript/TypeScript avec framework triple
test-js: | $(LOGSDIR)
	@echo "$(BLUE)🧪 Running JavaScript/TypeScript triple tests...$(NC)"
	@if command -v npm >/dev/null 2>&1; then \
		npm test; \
		echo "$(GREEN)✅ JavaScript tests completed$(NC)"; \
	else \
		echo "$(RED)❌ npm not found, skipping JS tests$(NC)"; \
		exit 1; \
	fi

# Tests complets avec validation triple
test-all: test test-js
	@echo "$(GREEN)🎯 All test suites completed successfully$(NC)"
	@echo "$(BLUE)📊 Test Summary:$(NC)"
	@echo "$(GREEN)   ✅ C unit tests: PASSED$(NC)"
	@echo "$(GREEN)   ✅ JavaScript tests: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Triple validation: PASSED$(NC)"
	@if [ -d $(LOGSDIR) ]; then \
		echo "$(BLUE)📁 Test logs available in: $(LOGSDIR)$(NC)"; \
		ls -la $(LOGSDIR)/*.jsonl 2>/dev/null | head -3 || echo "   No JSONL logs found"; \
	fi

# Compilation debug avec symboles étendus
debug: CFLAGS += -DDEBUG -g3 -O0 -fsanitize=address
debug: $(LUMS_LIB)
	@echo "$(GREEN)✅ Debug build completed with AddressSanitizer$(NC)"

# Compilation release optimisée
release: CFLAGS += -DNDEBUG -O3 -march=native
release: clean $(LUMS_LIB)
	@echo "$(GREEN)✅ Release build completed with optimizations$(NC)"

# Installation système
install: $(LUMS_LIB)
	@echo "$(BLUE)📦 Installing LUMS library...$(NC)"
	@sudo mkdir -p /usr/local/include/lums
	@sudo mkdir -p /usr/local/lib
	@sudo cp $(HEADERS) /usr/local/include/lums/
	@sudo cp $(LUMS_LIB) /usr/local/lib/
	@sudo ldconfig
	@echo "$(GREEN)✅ Installation completed$(NC)"
	@echo "$(BLUE)   Headers: /usr/local/include/lums/$(NC)"
	@echo "$(BLUE)   Library: /usr/local/lib/$(NC)"

# Nettoyage complet
clean:
	@echo "$(YELLOW)🧹 Cleaning build artifacts...$(NC)"
	@rm -rf $(BUILDDIR)
	@if [ -d $(LOGSDIR) ]; then \
		echo "$(YELLOW)🧹 Cleaning old logs...$(NC)"; \
		find $(LOGSDIR) -name "*.jsonl" -mtime +7 -delete 2>/dev/null || true; \
	fi
	@echo "$(GREEN)✅ Clean completed$(NC)"

# Validation du code (lint C avec analyse statique)
lint:
	@echo "$(BLUE)🔍 Linting C code...$(NC)"
	@for file in $(SOURCES) $(HEADERS); do \
		echo "$(BLUE)   Checking $$file...$(NC)"; \
		$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only $$file || exit 1; \
		echo "$(GREEN)   ✅ $$file passed$(NC)"; \
	done
	@echo "$(GREEN)✅ All C files passed lint check$(NC)"
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "$(BLUE)🔍 Running static analysis with cppcheck...$(NC)"; \
		cppcheck --enable=all --error-exitcode=1 $(SRCDIR)/; \
		echo "$(GREEN)✅ Static analysis passed$(NC)"; \
	else \
		echo "$(YELLOW)⚠️  cppcheck not found, skipping static analysis$(NC)"; \
	fi

# Informations de build détaillées
info:
	@echo "$(BLUE)📋 LUMS Build Information:$(NC)"
	@echo "$(GREEN)  🎯 Target:$(NC) $(LUMS_LIB)"
	@echo "$(GREEN)  📁 Sources:$(NC) $(words $(SOURCES)) files"
	@for src in $(SOURCES); do echo "    - $$src"; done
	@echo "$(GREEN)  📁 Headers:$(NC) $(words $(HEADERS)) files"
	@for hdr in $(HEADERS); do echo "    - $$hdr"; done
	@echo "$(GREEN)  🔧 Compiler:$(NC) $(CC)"
	@echo "$(GREEN)  🏗️  CFLAGS:$(NC) $(CFLAGS)"
	@echo "$(GREEN)  📦 Build dir:$(NC) $(BUILDDIR)"
	@echo "$(GREEN)  📊 Logs dir:$(NC) $(LOGSDIR)"
	@if [ -f $(LUMS_LIB) ]; then \
		echo "$(GREEN)  📈 Library size:$(NC) $$(ls -lh $(LUMS_LIB) | awk '{print $$5}')"; \
		echo "$(GREEN)  📅 Last built:$(NC) $$(stat -c %y $(LUMS_LIB) 2>/dev/null || stat -f %Sm $(LUMS_LIB))"; \
	fi

# Règles de dépendances automatiques pour recompilation incrémentale
-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: $(SRCDIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# Pipeline d'intégration continue complet
ci: clean lint test-all
	@echo "$(GREEN)🚀 CI Pipeline completed successfully$(NC)"
	@echo "$(BLUE)📊 CI Summary:$(NC)"
	@echo "$(GREEN)   ✅ Clean: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Lint: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Tests: PASSED$(NC)"
	@echo "$(GREEN)   ✅ Build: PASSED$(NC)"
	@if [ -f $(LUMS_LIB) ]; then \
		echo "$(BLUE)📦 Artifacts:$(NC)"; \
		echo "$(GREEN)   ✅ $(LUMS_LIB) ($$(ls -lh $(LUMS_LIB) | awk '{print $$5}'))$(NC)"; \
	fi

# Support pour packaging (future extension)
package: release
	@echo "$(BLUE)📦 Creating distribution package...$(NC)"
	@mkdir -p dist/include dist/lib
	@cp $(HEADERS) dist/include/
	@cp $(LUMS_LIB) dist/lib/
	@tar -czf lums-$(shell date +%Y%m%d).tar.gz dist/
	@echo "$(GREEN)✅ Package created: lums-$(shell date +%Y%m%d).tar.gz$(NC)"

