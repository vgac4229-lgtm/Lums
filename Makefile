
# Makefile pour compilation automatisée du code C LUMS
# Selon recommandations du rapport d'analyse

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -O2 -g
INCLUDES = -Iserver/lums
SRCDIR = server/lums
BUILDDIR = build
TESTDIR = tests

# Sources C
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
HEADERS = $(wildcard $(SRCDIR)/*.h)

# Binaires
LUMS_LIB = $(BUILDDIR)/liblums.a
TEST_BIN = $(BUILDDIR)/test_lums

.PHONY: all clean test install debug release

all: $(LUMS_LIB)

# Création du répertoire de build
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compilation des objets
$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Création de la bibliothèque statique
$(LUMS_LIB): $(OBJECTS)
	@echo "Creating static library..."
	ar rcs $@ $^
	@echo "Library created: $@"

# Tests unitaires C (si nécessaire)
test: $(LUMS_LIB)
	@echo "Running C unit tests..."
	@if [ -f $(TESTDIR)/test_lums.c ]; then \
		$(CC) $(CFLAGS) $(INCLUDES) $(TESTDIR)/test_lums.c -L$(BUILDDIR) -llums -o $(TEST_BIN); \
		./$(TEST_BIN); \
	else \
		echo "No C test file found, skipping C tests"; \
	fi

# Tests JavaScript/TypeScript
test-js:
	@echo "Running JavaScript/TypeScript tests..."
	npm test

# Tests complets (C + JS + triple validation)
test-all: test test-js
	@echo "All tests completed successfully"

# Compilation debug
debug: CFLAGS += -DDEBUG -g3
debug: $(LUMS_LIB)

# Compilation release
release: CFLAGS += -DNDEBUG -O3
release: clean $(LUMS_LIB)

# Installation (copie des headers et lib)
install: $(LUMS_LIB)
	@echo "Installing LUMS library..."
	mkdir -p /usr/local/include/lums
	mkdir -p /usr/local/lib
	cp $(HEADERS) /usr/local/include/lums/
	cp $(LUMS_LIB) /usr/local/lib/
	@echo "Installation completed"

# Nettoyage
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILDDIR)
	rm -f logs/*.jsonl
	@echo "Clean completed"

# Validation du code (lint C)
lint:
	@echo "Linting C code..."
	@for file in $(SOURCES) $(HEADERS); do \
		echo "Checking $$file..."; \
		$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only $$file; \
	done
	@echo "Lint completed"

# Informations de build
info:
	@echo "LUMS Build Information:"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Headers: $(HEADERS)"
	@echo "  Library: $(LUMS_LIB)"
	@echo "  CC: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"

# Règles de dépendances automatiques
-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: $(SRCDIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# Support pour intégration continue
ci: clean lint test-all
	@echo "CI pipeline completed successfully"
