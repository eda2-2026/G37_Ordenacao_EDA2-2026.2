

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
SRCDIR  = src
BIN     = livraria

SRC     = $(SRCDIR)/livro.c $(SRCDIR)/bucket.c $(SRCDIR)/main.c
OBJ     = $(SRC:.c=.o)
HEADERS = $(SRCDIR)/livro.h $(SRCDIR)/bucket.h

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) data/livros.csv

clean:
	rm -f $(OBJ) $(BIN) $(BIN).exe

.PHONY: all run clean
