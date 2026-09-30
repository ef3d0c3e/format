NAME := format
CC ?= cc
AR ?= ar
ARFLAGS ?= rcs
CFLAGS := -Wall -Wextra -Wconversion -pedantic -std=gnu23
IFLAGS := -I./include
LFLAGS :=

# Flags
EXTRA_CFLAGS ?= -ggdb
CFLAGS += $(EXTRA_CFLAGS)

SOURCES := \
	src/args.c \
	src/fmt.c \
	src/fmt_num.c \
	src/fmt_collection.c \
	src/fmt_char.c \
	src/buffer.c \
	src/fmt_str.c
OBJECTS := $(addprefix build/,$(SOURCES:.c=.o))

# -------
# Library
# -------

LIB := lib$(NAME).a

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

# -----
# Tests
# -----

TEST := build/test

# Definitions
TEST_SOURCES := $(shell find tests -name '*.c')
TEST_OBJECTS := $(addprefix build/,$(TEST_SOURCES:.c=.o))

CRITERION_SRC := libs/criterion
CRITERION_BUILD := build/criterion
CRITERION_PREFIX := $(CRITERION_BUILD)/install
CRITERION_PKGCONFIG := $(abspath $(CRITERION_PREFIX))/lib/pkgconfig
CRITERION_PC := $(CRITERION_PREFIX)/lib/pkgconfig/criterion.pc

# Test-only compiler flags
ifeq ($(findstring clang,$(notdir $(CC))),clang)
TEST_CFLAGS := -fmacro-backtrace-limit=0
else
TEST_CFLAGS := -ftrack-macro-expansion=0
endif

build/tests/%.o: tests/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) $(IFLAGS) -I$(CRITERION_SRC)/include -c $< -o $@

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
	$(CC) $(TEST_CFLAGS) $(CFLAGS) $(IFLAGS) \
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

# -------
# Doxygen
# -------

# Build doxygen
.PHONY: docs
docs:
	@mkdir -p docs
	doxygen doxygen

# ----
# Util
# ----

.PHONY: all
all: $(LIB)

.PHONY: clean
clean:
	$(RM) -r build/

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean all
