NAME := fmt
CC := gcc
CFLAGS := -Wall -Wextra -Wconversion -pedantic -ggdb -std=c99
IFLAGS := -I./include
LFLAGS :=

SOURCES := $(shell find src -name '*.c')
OBJECTS := $(addprefix build/,$(SOURCES:.c=.o))

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

# Default target
$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LFLAGS)

# Build doxygen
.PHONY: docs
docs:
	@mkdir -p docs
	doxygen doxygen

.PHONY: all
all: $(NAME)

.PHONY: clean
clean:
	$(RM) -r build/

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean all
