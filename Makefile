# ─────────────────────────────────────────────────────────────────────────────
# Makefile — MiniBrowser modular
# ─────────────────────────────────────────────────────────────────────────────

CC      := gcc
TARGET  := minibrowser
PKGS    := gtk+-3.0 webkit2gtk-4.1

CFLAGS  := $(shell pkg-config --cflags $(PKGS)) -Wall -Wextra -O2 -std=c11
LDFLAGS := $(shell pkg-config --libs   $(PKGS))

# Todos os .c do projeto
SRCS := main.c ui.c browser.c search.c homepage.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean deps run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✔  Build concluído → ./$(TARGET)"

# Regra genérica: compila cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependências explícitas de headers
main.o:     main.c     app.h ui.h browser.h
ui.o:       ui.c       app.h ui.h browser.h search.h
browser.o:  browser.c  app.h browser.h homepage.h search.h
search.o:   search.c   search.h
homepage.o: homepage.c homepage.h search.h

run: all
	./$(TARGET)

deps:
	sudo apt-get update
	sudo apt-get install -y \
		build-essential \
		libgtk-3-dev \
		libwebkit2gtk-4.1-dev \
		pkg-config

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "✔  Limpo."
