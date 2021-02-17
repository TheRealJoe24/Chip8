PROJECT_NAME := chip8

CC := gcc
WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline \
            -Wuninitialized -Wconversion -Wstrict-prototypes
CFLAGS := -g -std=c99 $(WARNINGS)
LDFLAGS ?=

.PHONY: all clean

SRC_FILES := $(wildcard src/*.c)
OBJ_FILES := $(SRC_FILES:.c=.o)
DEPFILES := $(SRC_FILES:.c=.d)

all: $(PROJECT_NAME)

run: all
	echo
	echo ----------------------------------------------------------------------
	echo 
	./$(PROJECT_NAME).out

test: run clean

$(PROJECT_NAME): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@.out

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm $(OBJ_FILES) $(DEPFILES) $(PROJECT_NAME).out

-include $(DEPFILES)
