# Makefile pour LUMS/VORAX Backend Complet
# Version: 2025.001 - Production Ready

# Configuration de base
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -fPIC -Wno-multichar -Wsign-compare
SRCDIR = server/lums
BUILDDIR = build
TESTDIR = tests

# Fichiers sources
SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(TESTDIR)/*.c)
OBJECTS = $(SOURCES:%.c=$(BUILDDIR)/%.o)
LIBRARY = $(BUILDDIR)/liblums.a

# Déclaration des objets LUMS pour la librairie
LUMS_OBJECTS = build/server/lums/decoder.o build/server/lums/encoder.o build/server/lums/operations.o \
               build/server/lums/vorax.o build/server/lums/lums_backend.o build/server/lums/electromechanical.o \
               build/server/lums/electromechanical_impl.o build/server/lums/advanced-math.o build/server/lums/lumgroup.o \
               build/server/lums/jit_compiler.o build/server/lums/vorax_simple.o build/server/lums/scientific_logger.o

# Configuration debug
DEBUG_FLAGS = -g3 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
RELEASE_FLAGS = -O3 -march=native -DNDEBUG

# Configuration pour profiling
PROFILE_FLAGS = -pg -fprofile-arcs -ftest-coverage

# Targets principaux
.PHONY: all clean debug release test info

# Target par défaut
all: $(LIBRARY)

# Configuration debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(LIBRARY)

# Configuration release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(LIBRARY)

# Configuration profiling
profile: CFLAGS += $(PROFILE_FLAGS)
profile: $(LIBRARY)

# Création répertoire build
$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(SRCDIR)
	mkdir -p $(BUILDDIR)/$(TESTDIR)
	mkdir -p build/server/lums
	mkdir -p build/tests
	mkdir -p logs/scientific_traces
	mkdir -p logs/performance
	mkdir -p logs/validation
	mkdir -p logs/memory
	mkdir -p logs/inspection_critique

# Compilation objets pour la librairie LUMS
build/server/lums/decoder.o: server/lums/decoder.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/encoder.o: server/lums/encoder.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/operations.o: server/lums/operations.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/vorax.o: server/lums/vorax.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/lums_backend.o: server/lums/lums_backend.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/electromechanical.o: server/lums/electromechanical.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/electromechanical_impl.o: server/lums/electromechanical_impl.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/advanced-math.o: server/lums/advanced-math.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/lumgroup.o: server/lums/lumgroup.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/jit_compiler.o: server/lums/jit_compiler.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/vorax_simple.o: server/lums/vorax_simple.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
build/server/lums/scientific_logger.o: server/lums/scientific_logger.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation objets pour les tests
$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Création librairie
$(LIBRARY): $(LUMS_OBJECTS)
	ar rcs $@ $^

# Exécutable test principal
$(BUILDDIR)/lums_test: $(BUILDDIR)/$(SRCDIR)/main_test.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILDDIR) -llums -lm -lpthread

# Console électromécanique
$(BUILDDIR)/electromechanical_console: $(BUILDDIR)/$(SRCDIR)/electromechanical_console.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILDDIR) -llums -lm -lpthread

# Serveur HTTP LUMS
$(BUILDDIR)/lums_http_server: $(BUILDDIR)/server/lums_http_server.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILDDIR) -llums -lm -lpthread

# Test stress 1M LUMs
$(BUILDDIR)/stress_test_1m_lums: $(BUILDDIR)/$(TESTDIR)/stress_test_1m_lums.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILDDIR) -llums -lm -lpthread

# Test validation scientifique
$(BUILDDIR)/scientific_validation: $(BUILDDIR)/$(TESTDIR)/scientific_validation_complete.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILDDIR) -llums -lm -lpthread

# Tests complets
test: $(BUILDDIR)/lums_test
	@echo "=== TESTS BACKEND LUMS COMPLET ==="
	./$(BUILDDIR)/lums_test

test-scientific: build/tests/scientific_validation_complete
	@echo "=== TESTS SCIENTIFIQUES COMPLETS ==="
	./build/tests/scientific_validation_complete

build/tests/scientific_validation_complete: tests/scientific_validation_complete.c $(LUMS_OBJECTS)
	@mkdir -p build/tests
	$(CC) $(CFLAGS) -o $@ $^ -lm -lpthread

# Test Node.js
test-js:
	npm test

# Tests de sécurité
test-security: debug
	@echo "=== TESTS SÉCURITÉ ==="
	valgrind --leak-check=full --show-leak-kinds=all ./$(BUILDDIR)/lums_test 2>&1 | tee logs/validation/security_$(shell date +%Y%m%d_%H%M%S).log

# Tests de performance
test-performance: release $(BUILDDIR)/stress_test_1m_lums
	@echo "=== TESTS PERFORMANCE ==="
	time ./$(BUILDDIR)/stress_test_1m_lums 2>&1 | tee logs/performance/stress_$(shell date +%Y%m%d_%H%M%S).log

# Tests avec Valgrind
test-valgrind: debug
	@echo "=== TESTS VALGRIND ==="
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./$(BUILDDIR)/lums_test 2>&1 | tee logs/validation/valgrind_$(shell date +%Y%m%d_%H%M%S).log

# Tests de stress
test-stress: $(BUILDDIR)/stress_test_1m_lums
	@echo "=== TESTS STRESS 1M LUMS ==="
	./$(BUILDDIR)/stress_test_1m_lums 2>&1 | tee logs/performance/stress_1m_$(shell date +%Y%m%d_%H%M%S).log

# Tests forensiques
test-forensic: $(BUILDDIR)/tests/scientific_validation_forensic
	@echo "=== VALIDATION FORENSIQUE COMPLÈTE ==="
	./build/tests/scientific_validation_forensic
	@echo "✅ Validation forensique terminée"

build/tests/scientific_validation_forensic: tests/scientific_validation_forensic.c $(LUMS_OBJECTS)
	@mkdir -p build/tests
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=199309L -o $@ $^ -lm -lpthread

# Développement backend complet
dev-backend: debug $(BUILDDIR)/electromechanical_console
	@echo "=== DÉVELOPPEMENT BACKEND LUMS ==="
	@echo "Console électromécanique disponible:"
	@echo "  ./$(BUILDDIR)/electromechanical_console"
	@echo ""
	@echo "Tests disponibles:"
	@echo "  make test          - Tests rapides"
	@echo "  make test-stress   - Tests 1M LUMs"
	@echo "  make test-valgrind - Tests mémoire"

# Stack complète
full-stack: release $(BUILDDIR)/lums_http_server
	@echo "=== STACK LUMS/VORAX COMPLÈTE ==="
	@echo "Démarrage serveur HTTP sur port 8080..."
	./$(BUILDDIR)/lums_http_server &
	@echo "PID serveur: $$!"
	@echo ""
	@echo "Démarrage interface Node.js..."
	npm run dev

# Tests de performance HTTP
perf-test: $(BUILDDIR)/lums_http_server
	@echo "=== TESTS PERFORMANCE HTTP ==="
	./$(BUILDDIR)/lums_http_server &
	@SERVER_PID=$$!; \
	sleep 2; \
	echo "Test GET /api/status:"; \
	curl -s -w "Time: %{time_total}s\n" http://localhost:8080/api/status; \
	echo ""; \
	echo "Test POST /api/fusion:"; \
	curl -s -w "Time: %{time_total}s\n" -X POST -H "Content-Type: application/json" -d '{"lum_a":"1010","lum_b":"1100"}' http://localhost:8080/api/fusion; \
	echo ""; \
	echo "Test POST /api/sqrt:"; \
	curl -s -w "Time: %{time_total}s\n" -X POST -H "Content-Type: application/json" -d '{"value":64}' http://localhost:8080/api/sqrt; \
	echo ""; \
	kill $$SERVER_PID

# Validation experte
expert-validation: test-security test-performance test-forensic
	@echo "=== VALIDATION EXPERTE COMPLÈTE ==="
	@echo "Tous les tests de validation ont été exécutés."
	@echo "Consultez les logs dans logs/validation/ pour analyse détaillée."

# Informations build
info:
	@echo "Build Configuration:"
	@echo "  CC: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Library: $(LIBRARY)"

# Nettoyage
clean:
	rm -rf $(BUILDDIR)
	rm -f *.o *.a
	rm -f logs/performance/*.log
	rm -f logs/validation/*.log

# Nettoyage complet
clean-all: clean
	rm -rf logs/scientific_traces/*.jsonl
	rm -rf logs/memory/*.lum

# Installation dépendances
install-deps:
	@echo "Installation des dépendances système..."
	sudo apt-get update || echo "apt-get non disponible"
	sudo apt-get install -y valgrind gdb || echo "Installation manuelle requise"

# Audit npm
audit:
	npm audit

# Correction audit npm
audit-fix:
	npm audit fix

# Vérification sécurité npm
security-check:
	npm audit --audit-level moderate

# Lint C (si disponible)
lint-c:
	@echo "Vérification style C..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 $(SRCDIR)/ 2>&1 | tee logs/validation/lint_$(shell date +%Y%m%d_%H%M%S).log; \
	else \
		echo "cppcheck non disponible - vérification manuelle"; \
	fi

# CI/CD pipeline complet
ci: clean lint-c debug test test-security audit-fix security-check
	@echo "=== PIPELINE CI/CD TERMINÉ ==="
	@echo "Vérifiez les logs pour validation complète"

# Aide
help:
	@echo "Targets disponibles:"
	@echo "  all              - Compilation standard"
	@echo "  debug            - Compilation debug avec sanitizers"
	@echo "  release          - Compilation optimisée"
	@echo "  test             - Tests rapides"
	@echo "  test-scientific  - Tests scientifiques"
	@echo "  test-forensic    - Validation scientifique forensique"
	@echo "  test-security    - Tests sécurité (Valgrind)"
	@echo "  test-performance - Tests performance (1M LUMs)"
	@echo "  test-stress      - Tests stress"
	@echo "  dev-backend      - Mode développement backend"
	@echo "  full-stack       - Stack complète (HTTP + UI)"
	@echo "  perf-test        - Tests performance HTTP"
	@echo "  expert-validation- Validation experte complète"
	@echo "  clean            - Nettoyage build"
	@echo "  clean-all        - Nettoyage complet"
	@echo "  ci               - Pipeline CI/CD complet"
	@echo "  generate_forensic_logs - Génère les logs forensiques corrigés"

display_logs:
	@echo "=== AFFICHAGE RÉSULTATS LOGS ==="
	@if [ -f logs/scientific_traces/lums_operations.jsonl ]; then \
		tail -2 logs/scientific_traces/lums_operations.jsonl; \
	else \
		echo "Aucun log disponible"; \
	fi

generate_forensic_logs:
	@echo "=== GÉNÉRATION LOGS FORENSIQUES CORRIGÉS ==="
	@chmod +x scripts/generate_forensic_logs.sh
	@./scripts/generate_forensic_logs.sh