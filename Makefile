NAME := fmt
CC := gcc
AR := ar
ARFLAGS := rcs
CFLAGS := -Wall -Wextra -Wconversion -pedantic -ggdb -std=gnu23
IFLAGS := -I./include
LFLAGS :=

SOURCES := $(shell find src -name '*.c')
OBJECTS := $(addprefix build/,$(SOURCES:.c=.o))

TEST_SOURCES := $(shell find tests -name '*.c')
TEST_OBJECTS := $(addprefix build/,$(TEST_SOURCES:.c=.o))

LIB := lib$(NAME).a
TEST := build/test

CRITERION_SRC := libs/criterion
CRITERION_BUILD := build/criterion
CRITERION_PREFIX := $(CRITERION_BUILD)/install
CRITERION_PKGCONFIG := $(abspath $(CRITERION_PREFIX))/lib/pkgconfig
CRITERION_PC := $(CRITERION_PREFIX)/lib/pkgconfig/criterion.pc

.PHONY: all
all: $(LIB)

# Default target
$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

build/tests/%.o: tests/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -ftrack-macro-expansion=2 $(IFLAGS) -I$(CRITERION_SRC)/include -c $< -o $@

# ---------
# Criterion
# ---------

.PHONY: criterion
criterion: $(CRITERION_PC)

$(CRITERION_BUILD)/build.ninja:
	meson setup $(CRITERION_BUILD) $(CRITERION_SRC) \
		--prefix=$(abspath $(CRITERION_PREFIX)) \
		--libdir=lib \
		-Ddefault_library=static

$(CRITERION_PC): $(CRITERION_BUILD)/build.ninja
	ninja -C $(CRITERION_BUILD) install

$(TEST): $(TEST_OBJECTS) $(LIB) $(CRITERION_PC)
	@mkdir -p $(@D)
	$(CC) -ftrack-macro-expansion=2 $(CFLAGS) $(IFLAGS) \
		$$(PKG_CONFIG_PATH=$(CRITERION_PKGCONFIG) pkg-config --static --cflags criterion) \
		-o $@ \
		$(TEST_OBJECTS) \
		$(LIB) \
		$$(PKG_CONFIG_PATH=$(CRITERION_PKGCONFIG) pkg-config --static --libs criterion)

.PHONY: test
test: $(TEST)
	./$(TEST)

.PHONY: test-vg
test-vg: $(TEST)
	valgrind --trace-children=yes ./$(TEST)

# Build doxygen
.PHONY: docs
docs:
	@mkdir -p docs
	doxygen doxygen

.PHONY: clean
clean:
	$(RM) -r build/

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean all
