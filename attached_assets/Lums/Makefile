# LUMS/VORAX Build System
# Automated compilation and testing pipeline

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC -O2
SRCDIR = server/lums
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
LIBRARY = $(BUILDDIR)/liblums.a

.PHONY: all clean test ci install

all: $(LIBRARY)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -rf $(BUILDDIR)

test: $(LIBRARY)
	npm test

ci: clean all test
	@echo "✅ Build pipeline completed successfully"

install: all
	@echo "✅ LUMS library built and ready"